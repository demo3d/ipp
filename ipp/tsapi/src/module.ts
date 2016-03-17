import * as emsc from './emscripten';


/**
 * @brief Typed JS proxies for Emscripten exported C functions
 */
export class CAPI {
    constructor(private _module: Module) {
        let cwrap = this._module.emscripten.cwrap;

        this.gl_context_initialize = cwrap('gl_context_initialize', 'number', ['string']);
        this.gl_context_activate = cwrap('gl_context_activate', null, ['number']);

        this.resource_register_fs_package = cwrap('resource_register_fs_package', null, ['number', 'string', 'string']);
        this.resource_register_ipparch_package = cwrap('resource_register_ipparch_package', null, ['number', 'string', 'string']);

        this.context_create = cwrap('context_create', 'number', ['string']);
        this.context_destroy = cwrap('context_destroy', null, ['number']);
        this.context_scene_create = cwrap('context_scene_create', 'number', ['number', 'string']);
        this.context_scene_destroy = cwrap('context_scene_destroy', null, ['number']);
        this.context_scene_get_loop = cwrap('context_scene_get_loop', 'number', ['number']);

        this.loop_create_event_listener = cwrap('loop_create_event_listener', 'number', ['number', 'number']);
        this.loop_release_listener = cwrap('loop_release_listener', null, ['number', 'number']);
        this.loop_find_message_type_id = cwrap('loop_find_message_type_id', 'number', ['number', 'string']);
        this.loop_find_message_type_name = cwrap('loop_find_message_type_name', 'string', ['number', 'number']);
        this.loop_message_get_type_id = cwrap('loop_message_get_type_id', 'number', ['number']);
        this.loop_message_get_type_name = cwrap('loop_message_get_type_name', 'string', ['number']);
        this.loop_enqueue_command = cwrap('loop_enqueue_command', null, ['number', 'number', 'number']);
        this.loop_update = cwrap('loop_update', null, ['number']);
    }

    gl_context_initialize: (target: string) => number = null;
    gl_context_activate: (handle: number) => void = null;

    resource_register_fs_package: (context: number, packageName: string, packagePath: string) => void = null;
    resource_register_ipparch_package: (context: number, packageName: string, packagePath: string) => number = null;

    context_create: (configuration: string) => number = null;
    context_destroy: (context: number) => void = null;
    context_scene_create: (context: number, path: string) => number = null;
    context_scene_destroy: (scene: number) => void;
    context_scene_get_loop: (scene: number) => number;

    loop_create_event_listener: (loop: number, callback: number) => number = null;
    loop_release_listener: (loop: number, listener: number) => void = null;
    loop_find_message_type_id: (loop: number, name: string) => number = null;
    loop_find_message_type_name: (loop: number, typeId: number) => string = null;
    loop_message_get_type_id: (messageReference: number) => number = null;
    loop_message_get_type_name: (message: number) => string = null;
    loop_enqueue_command: (loop: number, typeId: number, data: number) => void = null;
    loop_update: (loop: number) => void = null;

    /**
     * @brief Parent Module instance
     */
    get module(): Module {
        return this._module;
    }
}

/**
 * @brief Wrapper arround emscripten module object
 */
export class Module {
    private _capi: CAPI;

    constructor(
        private _emscripten: emsc.Module) {
        this._capi = new CAPI(this);
    }

    /**
     * @brief Emscripten javascript Module object
     */
    get emscripten(): emsc.Module { return this._emscripten; }

    /**
     * @brief Typed wrappers around emscripten exported C functions
     */
    get capi(): CAPI { return this._capi; }

    /**
     * @brief Allocate a MemoryBuffer with requested size on emscripten heap
     */
    allocateBuffer(size: number): MemoryBuffer {
        return new MemoryBuffer(this, this._emscripten._malloc(size), size, true);
    }


    getChar(ptr: number): number { return this._emscripten.HEAP8[ptr]; }
    getUChar(ptr: number): number { return this._emscripten.HEAPU8[ptr]; }
    getShort(ptr: number): number { return this._emscripten.HEAP16[ptr >> 1]; }
    getUShort(ptr: number): number { return this._emscripten.HEAPU16[ptr >> 1]; }
    getInt(ptr: number): number { return this._emscripten.HEAP32[ptr >> 2]; }
    getUInt(ptr: number): number { return this._emscripten.HEAPU32[ptr >> 2]; }
    getFloat32(ptr: number): number { return this._emscripten.HEAPF32[ptr >> 2]; }
    getFloat64(ptr: number): number { return this._emscripten.HEAPF64[ptr >> 3]; }
    setChar(ptr: number, value: number) { this._emscripten.HEAP8[ptr] = value; }
    setUChar(ptr: number, value: number) { this._emscripten.HEAPU8[ptr] = value; }
    setShort(ptr: number, value: number) { this._emscripten.HEAP16[ptr >> 1] = value; }
    setUShort(ptr: number, value: number) { this._emscripten.HEAPU16[ptr >> 1] = value; }
    setInt(ptr: number, value: number) { this._emscripten.HEAP32[ptr >> 2] = value; }
    setUInt(ptr: number, value: number) { this._emscripten.HEAPU32[ptr >> 2] = value; }
    setFloat32(ptr: number, value: number) { this._emscripten.HEAPF32[ptr >> 2] = value; }
    setFloat64(ptr: number, value: number) { this._emscripten.HEAPF64[ptr >> 3] = value; }
}


/**
 * @brief Typed access to primitive data from emscripten pointer to memory buffer
 */
export class MemoryBuffer {
    constructor(
        private _module: Module,
        private _pointer: number,
        private _size: number = -1,
        private _isOwned: boolean = false) {
    }

    /**
     * @brief Invalidate buffer memory reference
     * Free buffer memory if buffer object owns the memory
     */
    dispose() {
        if (this._isOwned) {
            this._module.emscripten._free(this._pointer);
            this._pointer = 0;
            this._size = -1;
        }
    }

    /**
     * @brief Module to which memory address space this buffer points to
     */
    get module(): Module {
        return this._module;
    }

    /**
     * @brief Buffer memory pointer
     */
    get pointer(): number {
        return this._pointer;
    }

    /**
     * @brief Buffer size in bytes, -1 if unknown
     */
    get size(): number {
        return this._size;
    }

    /**
     * @brief If true Buffer owns memory it points to and needs to be disposed to free it
     */
    get isOwned(): boolean {
        return this._isOwned;
    }


    getChar(offset: number): number { return this._module.getChar(this._pointer + offset); }
    getUChar(offset: number): number { return this._module.getUChar(this._pointer + offset); }
    getShort(offset: number): number { return this._module.getShort(this._pointer + offset); }
    getUShort(offset: number): number { return this._module.getUShort(this._pointer + offset); }
    getInt(offset: number): number { return this._module.getInt(this._pointer + offset); }
    getUInt(offset: number): number { return this._module.getUInt(this._pointer + offset); }
    getFloat32(offset: number): number { return this._module.getFloat32(this._pointer + offset); }
    getFloat64(offset: number): number { return this._module.getFloat64(this._pointer + offset); }
    setChar(offset: number, value: number) { this._module.setChar(this._pointer + offset, value); }
    setUChar(offset: number, value: number) { this._module.setUChar(this._pointer + offset, value); }
    setShort(offset: number, value: number) { this._module.setShort(this._pointer + offset, value); }
    setUShort(offset: number, value: number) { this._module.setUShort(this._pointer + offset, value); }
    setInt(offset: number, value: number) { this._module.setInt(this._pointer + offset, value); }
    setUInt(offset: number, value: number) { this._module.setUInt(this._pointer + offset, value); }
    setFloat32(offset: number, value: number) { this._module.setFloat32(this._pointer + offset, value); }
    setFloat64(offset: number, value: number) { this._module.setFloat64(this._pointer + offset, value); }

    /**
     * @brief Write all elements from valueArray as float32 starting at offset
     */
    setFloat32Array(offset: number, valueArray: number[]) {
        for (let value of valueArray) {
            this._module.setFloat32(this._pointer + offset, value);
            offset += 4;
        }
    }
}
