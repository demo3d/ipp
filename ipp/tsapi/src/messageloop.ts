import {Context} from './context';
import {Module, MemoryBuffer} from './module';


/**
 * @brief MessageLoop event with data pointer to Emscripten heap.
 * @note Data references are invalidated once the event handling function returns
 */
export class MessageLoopEvent<T> {
    constructor(
        private _messageLoop: MessageLoop,
        private _typeId: number,
        private _data: T) {
    }

    /**
     * @brief MessageLoop instance that distpatched the event
     */
    get messageLoop(): MessageLoop { return this._messageLoop; }

    /**
     * @brief Event Message unique type id number
     */
    get typeId(): number { return this._typeId; }

    /**
     * @brief Event data
     */
    get data(): T { return this._data; }

    /**
     * @brief Return a new MessageLoopEvent with same messageLoop/typeId but data produced by trans function.
     */
    transform<U>(trans: (src: MessageLoopEvent<T>) => U): MessageLoopEvent<U> {
        return new MessageLoopEvent<U>(this._messageLoop, this._typeId, trans(this));
    }
}

/**
 * @brief Event callback listener subscription object
 */
class Listener {
    constructor(
        private _typeId: number,
        public notify: (evt: MessageLoopEvent<MemoryBuffer>) => void) {
    }

    get typeId(): number { return this._typeId; }
}

/**
 * @brief MessageLoop wrapper interface
 */
export class MessageLoop {
    static COMMAND_MEMORY_BUFFER_SIZE: number = 1024 * 32;

    private _listenerReference: number;
    private _commandMemoryBuffer: MemoryBuffer;
    private _listeners: Array<Listener>;
    private _listenerCallback: (typeId: number, dataSize: number, data: number) => void = null;

    constructor(
        private _context: Context,
        private _reference: number) {
        this._commandMemoryBuffer = this.module.allocateBuffer(MessageLoop.COMMAND_MEMORY_BUFFER_SIZE);

        var listeners = new Array<Listener>();
        this._listeners = listeners;
        this._listenerCallback = (typeId: number, dataSize: number, data: number) => {
            let evt = new MessageLoopEvent<MemoryBuffer>(this, typeId, new MemoryBuffer(this.module, data, dataSize));
            for (let listener of listeners) {
                if (listener.typeId == typeId) {
                    listener.notify(evt);
                }
            }
        };
        this._listenerReference = this.module.emscripten.Runtime.addFunction(this._listenerCallback);
        this.module.capi.loop_create_event_listener(_reference, this._listenerReference);
    }

    /**
     * @brief Release event callback handle and close event stream
     * Resets all member references to null
     */
    dispose() {
        this.module.emscripten.Runtime.removeFunction(this._listenerReference);

        this._listenerCallback = null;
        this._listenerReference = null;
        this._listeners = null;

        this._commandMemoryBuffer.dispose();
        this._commandMemoryBuffer = null;

        this._context = null;
        this._reference = 0;
    }

    /**
     * @brief Enqueue a command to MessageLoop with specified  data pointer
     */
    enqueue(commandTypeId: number, data: number) {
        this.module.capi.loop_enqueue_command(this._reference, commandTypeId, data);
    }

    /**
     * @brief Find message type name associated with type id in MessageLoop
     * Empty string if no type associated with typeId or typeId is 0
     */
    findMessageName(typeId: number): string {
        if (typeId == 0) {
            return "";
        }
        return this.module.capi.loop_find_message_type_name(this._reference, typeId);
    }

    /**
     * @brief Find message type id associated with type name in MessageLoop
     * 0 if no type associated with type name or typeName is empty/null
     */
    findMessageId(typeName: string): number {
        if (typeName == "" || typeName == null) {
            return 0;
        }
        return this.module.capi.loop_find_message_type_id(this._reference, typeName);
    }

    /**
     * @brief Get message type id associated with type name in MessageLoop
     * Raises exception if typeName is not found or null/empty
     */
    getMessageId(typeName: string): number {
        var typeId = this.findMessageId(typeName);
        if (typeId == 0) {
            throw 'Message Type name not found in MessageLoop';
        }
        return typeId;
    }

    /**
     * @brief Update MessageLoop by performing an iteration
     */
    update() {
        this.module.capi.loop_update(this._reference);
    }

    /**
     * MemoryBuffer in Emscripten heap that can be reused by Command serialization API to
     * pass messages data pointer to MessageLoop (maximum size COMMAND_MEMORY_BUFFER_SIZE bytes).
     */
    get commandMemoryBuffer(): MemoryBuffer { return this._commandMemoryBuffer; }

    /**
     * @brief Parent Context instance
     */
    get context(): Context { return this._context; }

    /**
     * @brief Module access trough parent Context
     */
    get module(): Module { return this._context.module; }

    /**
     * @brief Subscribe callback to eventTypeId from message loop
     * @returns Unsubscribe closure that will unsubscribe the callback from eventTypeId
     */
    on(eventTypeId: number, callback: (evt: MessageLoopEvent<MemoryBuffer>) => void): () => void {
        let listener = new Listener(eventTypeId, callback);
        this._listeners.push(listener);
        return () => {
            this._listeners.splice(this._listeners.indexOf(listener));
        };
    }
}
