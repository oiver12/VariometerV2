import { Component, HostListener } from '@angular/core';

@Component({
    selector: 'app-home',
    templateUrl: './home.component.html',
    styleUrls: ['./home.component.css'],
})
export class HomeComponent {
    greyBottomPralax: number = 2.5;
    blackGreyBottomPralax: number = 2.3;

    constructor() {}
    AfterViewInit() {
        this.onResize(null);
    }
    @HostListener('window:resize', ['$event'])
    onResize(event: any) {
        // we dont want that the middle part crosses the lower part of the landing page.
        const middleDiv = document.getElementsByClassName(
            'middle-page'
        ) as HTMLCollectionOf<HTMLElement>;
        if (middleDiv[0]) {
            const upperLine = this.getAbsolutePosition(middleDiv[0]);

            const backgroundDiv = document.getElementById('background-div2');
            const bottomPage = document.getElementById('bottom-page');
            if (
                this.getBottomPosition() - upperLine.bottom <
                window.innerHeight * 1.25
            ) {
                const ratio =
                    (this.getBottomPosition() - upperLine.bottom) * 0.85;
                backgroundDiv!.style.height =
                    window.innerHeight * 4.25 - ratio + 'px';
                bottomPage!.style.top =
                    window.innerHeight * 3.33 - ratio + 'px';
                this.greyBottomPralax = 3.7 - ratio / window.innerHeight;
                this.blackGreyBottomPralax = 3.5 - ratio / window.innerHeight;
            } else {
                backgroundDiv!.style.height = '';
                bottomPage!.style.top = '';
                this.greyBottomPralax = 2.5;
                this.blackGreyBottomPralax = 2.3;
            }
        }
    }
    getBottomPosition(): number {
        const bottomDiv = document.getElementById(
            'background-div1'
        ) as HTMLElement;
        const bottomLine = this.getAbsolutePosition(bottomDiv);
        return bottomLine.bottom;
    }
    getAbsolutePosition(element: HTMLElement): { top: number; bottom: number } {
        const rect = element.getBoundingClientRect();
        const scrollTop = document.documentElement.scrollTop;
        const clientTop = document.documentElement.clientTop;

        const top = rect.top + scrollTop - clientTop;
        const bottom = top + rect.height;

        return { top, bottom };
    }
}
