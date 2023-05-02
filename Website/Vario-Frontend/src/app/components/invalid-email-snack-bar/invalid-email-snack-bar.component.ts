import { Component, inject } from '@angular/core';
import { MatSnackBarRef } from '@angular/material/snack-bar';

@Component({
  selector: 'app-invalid-email-snack-bar',
  templateUrl: './invalid-email-snack-bar.component.html',
  styleUrls: ['./invalid-email-snack-bar.component.css']
})
export class InvalidEmailSnackBarComponent {
      snackBarRef = inject(MatSnackBarRef);
}
