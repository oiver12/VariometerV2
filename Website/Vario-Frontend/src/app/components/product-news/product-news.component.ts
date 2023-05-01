import { Component } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';

@Component({
  selector: 'app-product-news',
  templateUrl: './product-news.component.html',
  styleUrls: ['./product-news.component.css']
})
export class ProductNewsComponent {

    constructor(private dialog: MatDialog) {}

    productNews() {
        this.dialog.open(ProductNewsComponent);
    }
}
