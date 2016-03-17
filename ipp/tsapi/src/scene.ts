import {Module, MemoryBuffer} from './module';
import {MessageLoop, MessageLoopEvent} from './messageloop';
import {CameraSystem} from './camera';
import {Context} from './context';


export enum AnimationStatus {
    Playing = 1,
    Stopped = 2,
    Completed = 3
}

/**
 * @event SceneAnimationStateUpdatedEvent
 *
 * Dispatched by animation system to notify other systems that system state has changed
 */
export class AnimationState {
    constructor(
        private _time: number,
        private _status: AnimationStatus) {
    }

    get time(): number { return this._time; }

    get status(): AnimationStatus { return this._status; }

    writeToBuffer(buffer: MemoryBuffer) {
        buffer.setUInt(0, this._time);
        buffer.setUInt(4, this._status);
    }

    static readFromBuffer(buffer: MemoryBuffer): AnimationState {
        return new AnimationState(buffer.getUInt(0), buffer.getUInt(4));
    }
}

/**
 * @command SceneRenderViewportResizeCommand
 * @event SceneRenderViewportResizedEvent
 *
 * Command render system that target render viewport should be resized
 */
export class RenderViewportSize {
    constructor(
        private _width: number,
        private _height: number) {
    }

    get width(): number { return this._width; }

    get height(): number { return this._height; }

    writeToBuffer(buffer: MemoryBuffer) {
        buffer.setUInt(0, this._width);
        buffer.setUInt(4, this._height);
    }

    static readFromBuffer(buffer: MemoryBuffer): RenderViewportSize {
        return new RenderViewportSize(buffer.getUInt(0), buffer.getUInt(4));
    }
}

/**
 * @brief Exposes Scene API and it's associated MessageLoop.
 */
export class Scene {
    private _commandAnimationPlay: number;
    private _commandAnimationStop: number;
    private _commandAnimationUpdate: number;
    private _commandRenderViewportResize: number;
    private _eventAnimationStateUpdated: number;
    private _eventRenderViewportResized: number;

    private _reference: number;
    private _messageLoop: MessageLoop;
    private _cameraSystem: CameraSystem;

    constructor(
        private _context: Context,
        private _resourcePath: string) {
        this._reference = this.module.capi.context_scene_create(_context.reference, _resourcePath);
        this._messageLoop = new MessageLoop(_context, _context.module.capi.context_scene_get_loop(this._reference));
        this._cameraSystem = new CameraSystem(this._messageLoop);

        // Initialize Event/Command state
        this._commandAnimationPlay = this._messageLoop.getMessageId('SceneAnimationPlayCommand');
        this._commandAnimationStop = this._messageLoop.getMessageId('SceneAnimationStopCommand');
        this._commandAnimationUpdate = this._messageLoop.getMessageId('SceneAnimationUpdateCommand');
        this._commandRenderViewportResize = this._messageLoop.getMessageId('SceneRenderViewportResizeCommand');
        this._eventAnimationStateUpdated = this._messageLoop.getMessageId('SceneAnimationStateUpdatedEvent');
        this._eventRenderViewportResized = this._messageLoop.getMessageId('SceneRenderViewportResizedEvent');
    }

    /**
     * @brief Dispose of Scene instance and any associated IPP handles
     */
    dispose() {
        this._cameraSystem.dispose();
        this._messageLoop.dispose();
        this.module.capi.context_scene_destroy(this._reference);

        this._cameraSystem = null;
        this._messageLoop = null;
        this._reference = 0;
    }

    /**
     * @brief Dispatch SceneAnimationPlayCommand with start/end time in milliseconds
     */
    animationPlay(start: number, end: number = 0) {
        let buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setUInt(0, start);
        buffer.setUInt(4, end);
        this._messageLoop.enqueue(this._commandAnimationPlay, buffer.pointer);
    }

    /**
     * @brief Dispatch SceneAnimationStopCommand
     */
    animationStop() {
        this._messageLoop.enqueue(this._commandAnimationStop, this._messageLoop.commandMemoryBuffer.pointer);
    }

    /**
     * @brief Dispatch SceneAnimationUpdateCommand with deltaTime in milliseconsd
     */
    animationUpdate(deltaTime: number) {
        let buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setUInt(0, deltaTime);
        this._messageLoop.enqueue(this._commandAnimationUpdate, buffer.pointer);
    }

    /**
     * @brief Register SceneAnimationStateUpdatedEvent callback 
     */
    onAnimationStateUpdated(callback: (state: MessageLoopEvent<AnimationState>) => void): () => void {
        return this._messageLoop.on(this._eventAnimationStateUpdated, (evt) => {
            callback(evt.transform((evt) => AnimationState.readFromBuffer(evt.data)));
        });
    }

    /**
     * @brief Dispatch SceneRenderViewportResizeCommand with new viewportSize
     */
    renderViewportResize(viewportSize: RenderViewportSize) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        viewportSize.writeToBuffer(buffer);
        this._messageLoop.enqueue(this._commandRenderViewportResize, buffer.pointer);
    }

    /**
     * @brief SceneRenderViewportResizedEvent exposed as a stream
     */
    onRenderViewportResized(callback: (camera: MessageLoopEvent<RenderViewportSize>) => void): () => void {
        return this._messageLoop.on(this._eventRenderViewportResized, (evt) => {
            callback(evt.transform((evt) => RenderViewportSize.readFromBuffer(evt.data)));
        });
    }

    /**
     * @brief CameraSystem proxy that interfaces with Scene CameraSystem trough messageLoop
     */
    get cameraSystem(): CameraSystem {
        return this._cameraSystem;
    }

    /**
     * @brief Path to the resource from which Scene object was created from
     */
    get resourcePath(): string {
        return this._resourcePath;
    }

    /**
     * @brief Scene object reference (pointer to Emscripten object)
     */
    get reference(): number {
        return this._reference;
    }

    /**
     * @brief Scene MessageLoop instance
     */
    get messageLoop(): MessageLoop {
        return this._messageLoop;
    }


    /**
     * @brief Context that created Scene
     */
    get context(): Context {
        return this._context;
    }

    /**
     * @brief Owning Module access trough context object
     */
    get module(): Module {
        return this._context.module;
    }
}
