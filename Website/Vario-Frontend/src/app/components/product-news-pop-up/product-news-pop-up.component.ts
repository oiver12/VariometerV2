import { Component } from '@angular/core';
import { MatSnackBar } from '@angular/material/snack-bar';
import { SubscribeToEmailService } from 'src/app/services/subscribe-to-email.service';
import { InvalidEmailSnackBarComponent } from '../invalid-email-snack-bar/invalid-email-snack-bar.component';
import { MatDialogRef } from '@angular/material/dialog';

@Component({
    selector: 'app-product-news-pop-up',
    templateUrl: './product-news-pop-up.component.html',
    styleUrls: ['./product-news-pop-up.component.css'],
})
export class ProductNewsPopUpComponent {
    constructor(
        public dialogRef: MatDialogRef<ProductNewsPopUpComponent>,
        private subscribeToEmailService: SubscribeToEmailService,
        private _snackBarRef: MatSnackBar
    ) {}

    onClick() {
        const expression: RegExp =
            /^(?=.{1,254}$)(?=.{1,64}@)[-!#$%&'*+/0-9=?A-Z^_`a-z{|}~]+(\.[-!#$%&'*+/0-9=?A-Z^_`a-z{|}~]+)*@[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?(\.[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$/;

        const email = (
            document.getElementsByClassName(
                'email-input'
            )[0] as HTMLInputElement
        ).value;
        if (expression.test(email)) {
            this.subscribeToEmailService.sendEmail(email);
            this.dialogRef.close();
        } else {
            this._snackBarRef.openFromComponent(InvalidEmailSnackBarComponent, {
                duration: 3000,
            });
        }
    }
}
