import {Module} from './module';
import {PackageManager, Package} from './package';


/**
 * @brief IPP library context that wraps resource manager/shared objects for Scenes
 *
 * Contains configuration settings used for configuring context shared behaviour (TODO)
 * and resource package sources searched when loading resources in the context.
 */
export class Context {
    private _reference: number;
    private _packageLoadRequests: Map<string, Promise<Package>>;
    private _glHandle: number;


    constructor(
        configuration: string,
        private _module: Module,
        private _packageManager: PackageManager,
        private _canvas: HTMLCanvasElement) {
        // create IPP context object
        this._reference = _module.capi.context_create(configuration);
        this._packageLoadRequests = new Map<string, Promise<Package>>();

        // initalize gl context handle for canvas
        this._module.emscripten.canvas = _canvas;
        this._glHandle = _module.capi.gl_context_initialize(null);
    }

    /**
     * @brief Release context referenced object and invalidate reference
     */
    dispose() {
        this._module.capi.context_destroy(this._reference);
        this._module = null;
        this._reference = 0;
        this._canvas = null;
        this._glHandle = null;
    }

    /**
     * @brief Request archive from resource provider and load it in emscripten FS
     */
    loadPackage(name: string): Promise<Package> {
        let FS = this._module.emscripten.FS;
        let request = this._packageLoadRequests.get(name);
        if (request != null) {
            return request;
        }

        let pkg = this._packageManager.findPackage(name);
        if (pkg == null) {
            throw `Package ${name} not found in PackageManager`;
        }

        request = pkg.data.then((data) => {
            // create path directories
            let packagePath = `resources/${name}.ipparch`;
            let path = '';
            let pathComponents = packagePath.split('/');
            for (let component of pathComponents.slice(0, pathComponents.length - 1)) {
                if (component == '') {
                    continue;
                }
                path = `${path}/${component}`;
                try {
                    FS.stat(path);
                } catch (_) {
                    FS.mkdir(path);
                }
            }

            // write data to FS
            FS.writeFile(packagePath, new Uint8Array(data), { encoding: 'binary', flags: 'w' });

            // register package to context
            this._module.capi.resource_register_ipparch_package(this._reference, name, packagePath);
            return pkg;
        });
        this._packageLoadRequests.set(name, request);
        return request;
    }

    /**
     * @brief Context PackageManager instance used to load package from
     */
    get packageManager(): PackageManager { return this._packageManager; }

    /**
     * @brief Context object reference (pointer to Emscripten object)
     */
    get reference(): number { return this._reference; }

    /**
       * @brief Emscripten handle to GL context for this Context
       */
    get glHandle(): number { return this._glHandle; }

    /**
     * @brief DOM canvas that owns the GL context for this Context instance
     */
    get canvas(): HTMLCanvasElement { return this._canvas; }

    /**
     * @brief Context parent Module instance
     */
    get module(): Module { return this._module; }
}
