import { Directive, Input, ElementRef, HostListener } from '@angular/core';

@Directive({
    selector: '[appParallax]',
})
export class ParallaxDirective {
    @Input('ratio') parallaxRatio: number = 1;
    @Input('top') initialTop: number = 0;
    top: number = 0;

    constructor(private eleRef: ElementRef) {}

    @HostListener('window:scroll', ['$event'])
    onWindowScroll(event: any) {
        this.top = window.innerHeight * this.initialTop;
        this.eleRef.nativeElement.style.top =
            this.top - window.scrollY * this.parallaxRatio + 'px';
    }
    @HostListener('window:resize', ['$event'])
    onWindowResize(event: any) {
        this.onWindowScroll(event);
    }
}
