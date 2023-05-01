import { Component } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { ProductNewsPopUpComponent } from '../product-news-pop-up/product-news-pop-up.component';

@Component({
  selector: 'app-product-news',
  templateUrl: './product-news.component.html',
  styleUrls: ['./product-news.component.css']
})
export class ProductNewsComponent {

    constructor(private dialog: MatDialog) {}
    ngOnInit(): void {
        this.productNews()
    }

    productNews() {
        const dialogRef = this.dialog.open(ProductNewsPopUpComponent, {
            width: '40rem', height: '500px'
        });
    }
}
 