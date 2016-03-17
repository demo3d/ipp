import {MessageLoop, MessageLoopEvent} from './messageloop';
import {MemoryBuffer} from './module';
import {Vector3D, NumberLimits, Vector3DLimits} from './primitive';


export enum CameraType {
    UserControlled = 1,
    Entity = 2
}

/**
 * @command SceneCameraUserControlledStateSetCommand
 * @event SceneCameraUserControlledStateUpdatedEvent
 *
 * Camera state data, can be dispatched as a Command to update user controlled Camera or
 * received as an event every time state is modified
 */
export class CameraUserControlledState {
    constructor(
        private _position: Vector3D,
        private _target: Vector3D,
        private _up: Vector3D) {
    }

    /**
     * @brief Camera world space position - eye position
     */
    get position(): Vector3D { return this._position; }

    /**
     * @brief Camera view target (view direction and point arround which camera is rotated)
     */
    get target(): Vector3D { return this._target; }

    /**
     * @brief Camera up vector (normalized)
     */
    get up(): Vector3D { return this._up; }

    writeToBuffer(buffer: MemoryBuffer) {
        buffer.setFloat32Array(0, [
            this.position.x, this.position.y, this.position.z,
            this.target.x, this.target.y, this.target.z,
            this.up.x, this.up.y, this.up.z]);
    }

    static readFromBuffer(buffer: MemoryBuffer): CameraUserControlledState {
        return new CameraUserControlledState(
            { x: buffer.getFloat32(0), y: buffer.getFloat32(4), z: buffer.getFloat32(8) },
            { x: buffer.getFloat32(12), y: buffer.getFloat32(16), z: buffer.getFloat32(20) },
            { x: buffer.getFloat32(24), y: buffer.getFloat32(28), z: buffer.getFloat32(32) });
    }
}

/**
 * @command SceneCameraUserControlledLimitsSetCommand
 * @event SceneCameraUserControlledLimitsUpdatedEvent
 *
 * User controlled camera limits for specific values.
 */
export class UserControlledCameraLimits {
    constructor(
        private _target: Vector3DLimits = { x: { min: Number.MIN_VALUE, max: Number.MAX_VALUE }, y: { min: Number.MIN_VALUE, max: Number.MAX_VALUE }, z: { min: Number.MIN_VALUE, max: Number.MAX_VALUE } },
        private _position: Vector3DLimits = { x: { min: Number.MIN_VALUE, max: Number.MAX_VALUE }, y: { min: Number.MIN_VALUE, max: Number.MAX_VALUE }, z: { min: Number.MIN_VALUE, max: Number.MAX_VALUE } },
        private _distance: NumberLimits = { min: 1, max: 10 },
        private _rotationPolarUpAngleCos: NumberLimits = { min: -1, max: 1 }) {
    }

    /**
     * @brief Min/max camera target vector (point eye is looking at and arround which it hovers)
     */
    get target(): Vector3DLimits { return this._target; }

    /**
     * @brief Min/max camera position vector (eye location)
     */
    get position(): Vector3DLimits { return this._position; }

    /**
     * @brief Min/max distance between eye position and view target
     */
    get distance(): NumberLimits { return this._distance; }

    /**
     * @brief Min/max angle cosine (dot product) between global up vector (0,0,1) and view direction
     *
     * This constraint is only applied when performing polar rotation
     */
    get rotationPolarUpAngleCos(): NumberLimits { return this._rotationPolarUpAngleCos; }

    writeToBuffer(buffer: MemoryBuffer) {
        buffer.setFloat32Array(0, [
            this.target.x.min, this.target.y.min, this.target.z.min,
            this.target.x.max, this.target.y.max, this.target.z.max,

            this.position.x.min, this.position.y.min, this.position.z.min,
            this.position.x.max, this.position.y.max, this.position.z.max,

            this.distance.min, this.distance.max,
            this.rotationPolarUpAngleCos.max, this.rotationPolarUpAngleCos.max]);
    }

    static readFromBuffer(buffer: MemoryBuffer): UserControlledCameraLimits {
        return new UserControlledCameraLimits(
            // target
            {
                x: { min: buffer.getFloat32(0), max: buffer.getFloat32(12) },
                y: { min: buffer.getFloat32(4), max: buffer.getFloat32(16) },
                z: { min: buffer.getFloat32(8), max: buffer.getFloat32(20) }
            },

            // position
            {
                x: { min: buffer.getFloat32(24), max: buffer.getFloat32(36) },
                y: { min: buffer.getFloat32(28), max: buffer.getFloat32(40) },
                z: { min: buffer.getFloat32(32), max: buffer.getFloat32(44) }
            },

            // distance
            { min: buffer.getFloat32(48), max: buffer.getFloat32(52) },

            // rotation polar up angle axis
            { min: buffer.getFloat32(56), max: buffer.getFloat32(60) }
        )
    }
}


/**
 * @brief Wraps CameraUserControlledSystem
 *
 * IPP CameraSystem has a single CameraUserControlledSystem instance that exposes
 * an API for user controlled camera navigation wrapped by this class.
 */
export class CameraUserControlledSystem {
    private _commandRotatePolar: number;
    private _commandRotationArcballStart: number;
    private _commandRotationArcballUpdate: number;
    private _commandZoom: number;
    private _commandMove: number;
    private _commandStateSet: number;
    private _commandLimitsSet: number;
    private _eventStateUpdated: number;
    private _eventLimitsUpdated: number;

    constructor(private _messageLoop: MessageLoop) {
        this._commandMove = _messageLoop.getMessageId('SceneCameraUserControlledMoveCommand');
        this._commandRotatePolar = _messageLoop.getMessageId('SceneCameraUserControlledRotatePolarCommand');
        this._commandRotationArcballStart = _messageLoop.getMessageId('SceneCameraUserControlledRotationArcballStartCommand');
        this._commandRotationArcballUpdate = _messageLoop.getMessageId('SceneCameraUserControlledRotationArcballUpdateCommand');
        this._commandZoom = _messageLoop.getMessageId('SceneCameraUserControlledZoomCommand');
        this._commandStateSet = _messageLoop.getMessageId('SceneCameraUserControlledStateSetCommand');
        this._commandLimitsSet = _messageLoop.getMessageId('SceneCameraUserControlledLimitsSetCommand');
        this._eventStateUpdated = _messageLoop.getMessageId('SceneCameraUserControlledStateUpdatedEvent');
        this._eventLimitsUpdated = _messageLoop.getMessageId('SceneCameraUserControlledLimitsUpdatedEvent');
    }

    /**
     * @brief Dispatch RotatePolarCommand with delta latitude/longitude in degrees
     */
    rotatePolar(deltaLatitude: number, deltaLongitude: number) {
        let buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setFloat32Array(0, [deltaLatitude, deltaLongitude]);
        this._messageLoop.enqueue(this._commandRotatePolar, buffer.pointer);
    }

    /**
     * @brief Dispatch RotationArcballStart with x/y being normalized arcball coordinates
     */
    rotationArcballStart(x: number, y: number) {
        let buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setFloat32Array(0, [x, y]);
        this._messageLoop.enqueue(this._commandRotationArcballStart, buffer.pointer);
    }

    /**
     * @brief Dispatch RotationArcballUpdate with x/y being normalized arcball coordinates
     *
     * Arcball rotation is done by projecting x/y to unit sphere and rotating the camera by
     * the angle between previous point and new one.
     */
    rotationArcballUpdate(x: number, y: number) {
        let buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setFloat32Array(0, [x, y]);
        this._messageLoop.enqueue(this._commandRotationArcballUpdate, buffer.pointer);
    }

    /**
     * @brief Dispatch ZoomCommand with deltaDistance
     */
    zoom(deltaDistance: number) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setFloat32(0, deltaDistance);
        this._messageLoop.enqueue(this._commandZoom, buffer.pointer);
    }

    /**
     * @brief Dispatch MoveCommand with deltaX/Y/Z
     * Moves the camera relative to view direction, X is left/right, Y is up/down, Z is forward/back
     */
    move(deltaX: number, deltaY: number, deltaZ: number) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setFloat32Array(0, [deltaX, deltaY, deltaZ]);
        this._messageLoop.enqueue(this._commandMove, buffer.pointer);
    }

    /**
     * @brief Dispatch StateSetCommand with new state
     */
    setState(state: CameraUserControlledState) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        state.writeToBuffer(buffer);
        this._messageLoop.enqueue(this._commandStateSet, buffer.pointer);
    }

    /**
     * @brief Register a callback to StateUpdatedEvent
     * @returns Closure that will unsubscribe callback from Camera when called
     */
    onStateUpdated(callback: (state: MessageLoopEvent<CameraUserControlledState>) => void): () => void {
        return this._messageLoop.on(this._eventStateUpdated, (evt) => {
            callback(evt.transform((evt) => CameraUserControlledState.readFromBuffer(evt.data)));
        });
    }

    /**
     * @brief Dispatch LimitsSetCommand with new limits
     */
    setLimits(limits: UserControlledCameraLimits) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        limits.writeToBuffer(buffer);
        this._messageLoop.enqueue(this._commandLimitsSet, buffer.pointer);
    }

    /**
     * @brief Register a callback to LimitsUpdatedEvent
     * @returns Closure that will unsubscribe callback from Camera when called
     */
    onLimitsUpdated(callback: (state: MessageLoopEvent<UserControlledCameraLimits>) => void): () => void {
        return this._messageLoop.on(this._eventLimitsUpdated, (evt) => {
            callback(evt.transform((evt) => UserControlledCameraLimits.readFromBuffer(evt.data)));
        });
    }

    /**
     * @brief MessageLoop instance used to communicate with camera system
     */
    get messageLoop(): MessageLoop { return this._messageLoop; }
}


/**
 * @brief Wraps IPP CameraNodeSystem
 */
export class CameraNodeSystem {
    private _commandNodeActiveSet: number;
    private _eventNodeActiveUpdated: number;

    constructor(private _messageLoop: MessageLoop) {
        this._commandNodeActiveSet = _messageLoop.getMessageId('SceneCameraNodeActiveSetCommand');
        this._eventNodeActiveUpdated = _messageLoop.getMessageId('SceneCameraNodeActiveUpdatedEvent');
    }

    /**
     * @brief Dispatch NodeActiveSet with entityId for new active entity camera node
     */
    setActiveEntity(entityId: number) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setUInt(entityId, 0);
        this._messageLoop.enqueue(this._commandNodeActiveSet, buffer.pointer);
    }

    /**
     * @brief Register a callback to NodeActiveUpdated
     * @returns Closure that will unsubscribe callback from Camera when called
     */
    onActiveEntityUpdated(callback: (state: MessageLoopEvent<number>) => void): () => void {
        return this._messageLoop.on(this._eventNodeActiveUpdated, (evt) => {
            callback(evt.transform((evt) => evt.data.getUInt(0)));
        });
    }

    /**
     * @brief MessageLoop instance used to communicate with camera system
     */
    get messageLoop(): MessageLoop { return this._messageLoop; }
}

/**
 * @brief Wrapper arround IPP CameraSystem
 *
 * Determines which Camera subsystem is currently active and holds system proxy object instances
 */
export class CameraSystem {
    private _commandTypeActiveSet: number;
    private _eventTypeActiveUpdated: number;

    private _userControlledCamera: CameraUserControlledSystem;
    private _nodeCamera: CameraNodeSystem;

    private _activeCameraType: CameraType;
    private _activeCameraTypeUnsubscribe: () => void;

    constructor(
        private _messageLoop: MessageLoop) {
        this._commandTypeActiveSet = _messageLoop.getMessageId('SceneCameraActiveTypeSetCommand');
        this._eventTypeActiveUpdated = _messageLoop.getMessageId('SceneCameraActiveTypeUpdatedEvent');

        this._userControlledCamera = new CameraUserControlledSystem(_messageLoop);
        this._nodeCamera = new CameraNodeSystem(_messageLoop);

        this._activeCameraTypeUnsubscribe = this._messageLoop.on(this._eventTypeActiveUpdated, (evt) => {
            this._activeCameraType = evt.data.getUInt(0);
        });
    }

    /**
     * @brief Dispose of internal handles and invalidate object
     */
    dispose() {
        this._userControlledCamera = null;
        this._nodeCamera = null;
        this._messageLoop = null;
        this._activeCameraType = null;
        this._activeCameraTypeUnsubscribe();
        this._activeCameraTypeUnsubscribe = null;
    }

    /**
     * @brief Last active camera type
     */
    get activeCameraType(): CameraType { return this._activeCameraType; }

    /**
     * @brief Dispatch TypeActiveSet with camera type for new active camera system
     */
    setActiveType(type: CameraType) {
        var buffer = this._messageLoop.commandMemoryBuffer;
        buffer.setUInt(0, type);
        this._messageLoop.enqueue(this._commandTypeActiveSet, buffer.pointer);
    }

    /**
     * @brief Register a callback to TypeActiveUpdated
     * @returns Closure that will unsubscribe callback from Camera when called
     */
    onActiveTypeUpdated(callback: (state: MessageLoopEvent<CameraType>) => void): () => void {
        return this._messageLoop.on(this._eventTypeActiveUpdated, (evt) => {
            callback(evt.transform((evt) => evt.data.getUInt(0)));
        });
    }

    /**
     * @brief User controlled camera system instance for this CameraSystem
     */
    get userControlledCamera(): CameraUserControlledSystem { return this._userControlledCamera; }

    /**
     * @brief Node camera system instance for this CameraSystem
     */
    get nodeCamera(): CameraNodeSystem { return this._nodeCamera; }

    /**
     * @brief MessageLoop instance used to communicate with camera system
     */
    get messageLoop(): MessageLoop { return this._messageLoop; }
}
