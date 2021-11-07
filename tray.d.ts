declare module 'ctray' {
    namespace ctray {
        type MenuItem = string | {
            text: string;
            callback?: () => void;
            checked?: boolean;
            disabled?: boolean;
            submenu?: MenuItem[];
        };

        class Tray extends events.EventEmitter {
            constructor(icon: string);
            close(): void;

            icon: string;
            menu: MenuItem[];
            tooltip: string;
        };
    }

    export = ctray.Tray;
}
