declare module 'ctray' {
    namespace ctray {
        type MenuItem = string | {
            text: string;
            callback?: () => void;
            checked?: boolean;
            disabled?: boolean;
            submenu?: MenuItem[];
        };

        class Tray {
            constructor(icon: string, menu: MenuItem[]);
            
            start(): Promise<>;
            stop(): void;
            update(): void;
            
            
            icon: string;
            menu: MenuItem[];
        };
    }

    export = ctray.Tray;
}
