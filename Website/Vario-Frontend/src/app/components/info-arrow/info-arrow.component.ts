import { Component, HostListener } from '@angular/core';

@Component({
    selector: 'app-info-arrow',
    templateUrl: './info-arrow.component.html',
    styleUrls: ['./info-arrow.component.css'],
})
export class InfoArrowComponent {
    scrollDown() {
        window.scrollTo({
            top: document.getElementsByClassName('right-main-div')[0]
                .clientHeight,
            behavior: 'smooth',
        });
    }
    @HostListener('window:scroll')
    onScroll() {
        const inforArrow = document.getElementById('infoArrow');
        if (inforArrow) {
            if (window.scrollY / window.outerHeight > 0.2) {
                inforArrow.style.opacity = '0';
            } else {
                inforArrow.style.opacity = `${
                    1 - (window.scrollY / window.outerHeight) * 4
                }`;
            }
        }
    }
}
