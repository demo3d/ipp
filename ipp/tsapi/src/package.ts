export class Package {
    private _data: Promise<ArrayBuffer> = null;

    constructor(
        private _name: string,
        private _sourceUrl: string) {

    }

    /**
     * @brief Package unique name identifier.
     */
    get name(): string { return this._name; }

    /**
     * @brief Package source URL
     */
    get sourceUrl(): string { return this._sourceUrl; }

    /**
     * @brief Access package data byte buffer as a Future
     *
     * If the package is currently downloading or already downloaded existing/completed Future
     * object will be returned, otherwise a new HttpRequest will be created to sourceUrl
     */
    get data(): Promise<ArrayBuffer> {
        if (this._data == null) {
            this._data = fetch(this._sourceUrl).then((request) => request.arrayBuffer());
        }
        return this._data;
    }
}

export class PackageManager {
    private _packages: Array<Package>;

    constructor() {
        this._packages = new Array<Package>();
    }

    /**
     * @brief Create a Package object with name and data at sourceUrl
     * @note Does not start downloading Package data
     */
    registerPackage(name: string, sourceUrl: string) {
        let pkg = this._packages.find((pkg) => pkg.name == name);
        if (pkg) {
            if (pkg.sourceUrl != null) {
                throw `Cannot register a package ${name} at ${sourceUrl}, package already registered with ${pkg.sourceUrl}`;
            }
            return;
        }
        this._packages.push(new Package(name, sourceUrl));
    }

    /**
     * @brief Find a package by name
     * Returns null if no package with that name registered
     */
    findPackage(name: string): Package {
        return this._packages.find((pkg) => pkg.name == name);
    }

    /**
     * @brief Package sources
     */
    get packages(): Array<Package> { return this._packages; }
}
