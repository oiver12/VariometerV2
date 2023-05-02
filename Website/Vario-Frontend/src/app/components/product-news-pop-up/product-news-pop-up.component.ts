import { Component } from '@angular/core';
import { FormControl, Validators } from '@angular/forms';

@Component({
    selector: 'app-product-news-pop-up',
    templateUrl: './product-news-pop-up.component.html',
    styleUrls: ['./product-news-pop-up.component.css'],
})
export class ProductNewsPopUpComponent {
    onClick() {
        const expression: RegExp =
            /^(?=.{1,254}$)(?=.{1,64}@)[-!#$%&'*+/0-9=?A-Z^_`a-z{|}~]+(\.[-!#$%&'*+/0-9=?A-Z^_`a-z{|}~]+)*@[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?(\.[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$/;

        const email = (
            document.getElementsByClassName(
                'email-input'
            )[0] as HTMLInputElement
        ).value;
        if(expression.test(email)) {
            alert('Thank you for subscribing!');
        }
    }
}
