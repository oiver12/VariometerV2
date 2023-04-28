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
            this.rotate(0, div, angle);
        });
        window.onload = () => {
            for (var i = 0, ii = divs.length; i < ii; i++) {
                divs[i].style.transition = 'transform 2s ease-in-out';
            }
        };
        setInterval(() => {
            if (document.hidden) return;
            this.degrees += 60;
            divs.forEach((div: HTMLElement, index: number) => {
                const angle = index * 60;
                this.rotate(this.degrees + angle, div);
            });
        }, 5000);
    }

    rotate(degrees: number, div: HTMLElement, angle?: number) {
        const finalAngle = angle !== undefined ? angle : 0;
        this.renderer.setStyle(
            div,
            'transform',
            `rotate(${degrees + finalAngle}deg)`
        );
    }
}
