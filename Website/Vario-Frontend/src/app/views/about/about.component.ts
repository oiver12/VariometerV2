import { Component } from '@angular/core';

@Component({
    selector: 'app-about',
    templateUrl: './about.component.html',
    styleUrls: ['./about.component.css'],
})
export class AboutComponent {
    ngOnInit() {
        // change the background color of the body
        document.body.style.setProperty('--left-bkg-color', '#f5f5f5');
        document.body.style.setProperty('--text-color', '#000000');
    }
}
