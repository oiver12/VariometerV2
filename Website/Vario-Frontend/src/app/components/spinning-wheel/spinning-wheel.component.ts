import { Component, ElementRef, Renderer2 } from '@angular/core';

@Component({
    selector: 'app-spinning-wheel',
    templateUrl: './spinning-wheel.component.html',
    styleUrls: ['./spinning-wheel.component.css'],
})
export class SpinningWheelComponent {
    degrees = 0;
    properties = [
        'Swiss Made',
        'Robust',
        'Sensitiv',
        'Akkurat',
        'Reaktionsschnell',
        'Kompakt',
    ];

    constructor(private elRef: ElementRef, private renderer: Renderer2) {}

    ngAfterViewInit() {
        const divs = this.elRef.nativeElement.querySelectorAll('.rotate');

        for (var i = 0, ii = divs.length; i < ii; i++) {
            divs[i].style.transition = 'none';
        }
        divs.forEach((div: HTMLElement, index: number) => {
            const angle = index * 60;
            this.rotate(angle, div);
        });
        const transitionTime = 3;
        const wayOfTransition = 'ease-in-out';
        const everyXSeconds = 6;
        window.onload = () => {
            for (var i = 0, ii = divs.length; i < ii; i++) {
                divs[i].style.transition = 'transform ' + transitionTime.toString() + 's ' + wayOfTransition;
            }
            this.degrees += 60;
            divs.forEach((div: HTMLElement, index: number) => {
                const angle = index * 60;
                this.rotate(this.degrees + angle, div);
            });
        };
        setInterval(() => {
            if (document.hidden) {
                return;
            }
            this.degrees += 60;
            divs.forEach((div: HTMLElement, index: number) => {
                const angle = index * 60;
                this.rotate(this.degrees + angle, div);
            });
        }, everyXSeconds * 1000);
    }

    rotate(degrees: number, div: HTMLElement) {
        this.renderer.setStyle(
            div,
            'transform',
            `rotate(${degrees}deg)`
        );
    }
}
