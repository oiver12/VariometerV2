import { Component, ElementRef, ViewChild } from '@angular/core';

@Component({
    selector: 'app-info-arrow',
    templateUrl: './info-arrow.component.html',
    styleUrls: ['./info-arrow.component.css'],
})
export class InfoArrowComponent {
    @ViewChild('infoArrow') infoArrow!: ElementRef;

    ngAfterViewInit() {
        console.log(this.infoArrow);
        // const elementPosition =
        //     this.infoArrow.nativeElement.getBoundingClientRect();
        // console.log(elementPosition);
    }
}
