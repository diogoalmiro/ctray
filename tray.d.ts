declare module "ctray" {
    type MenuItem = string | {
        text: string;
        callback?: () => void;
        checked?: boolean;
        disabled?: boolean;
        submenu?: MenuItem[];
    };
    
    export = Tray;
    
    declare class Tray {
        constructor(icon: string, menu: MenuItem[]);
        
        start(): Promise<>;
        stop(): void;
        update(): void;
        
        
        icon: string;
        menu: MenuItem[];
    }
}
