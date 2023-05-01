import { Component } from '@angular/core';
import { FormControl, Validators } from '@angular/forms';

@Component({
    selector: 'app-product-news-pop-up',
    templateUrl: './product-news-pop-up.component.html',
    styleUrls: ['./product-news-pop-up.component.css'],
})
export class ProductNewsPopUpComponent {
    email = new FormControl('', [Validators.required, Validators.email]);
    inputValue: string = '';

    getErrorMessage() {
        if (this.email.hasError('required')) {
            return 'Bitte geben Sie eine E-Mail-Adresse ein';
        }

        return this.email.hasError('email') ? 'Bitte geben Sie eine valide E-Mail-Adresse ein' : '';
    }
}
