import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { MaterialModule } from './material/material.module';
import { ThreejsComponent } from './components/threejs/threejs.component';
import { SpinningWheelComponent } from './components/spinning-wheel/spinning-wheel.component';
import { HomeComponent } from './views/home/home.component';
import { InfoTextComponent } from './components/info-text/info-text.component';
import { ProductNewsComponent } from './components/product-news/product-news.component';
import { ProductNewsPopUpComponent } from './components/product-news-pop-up/product-news-pop-up.component';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import { InvalidEmailSnackBarComponent } from './components/invalid-email-snack-bar/invalid-email-snack-bar.component';
import { InfoArrowComponent } from './components/info-arrow/info-arrow.component';
import { FirstSideComponent } from './components/first-side/first-side.component';
import { SecondSideComponent } from './components/second-side/second-side.component';
import { LongInfoTextComponent } from './components/long-info-text/long-info-text.component';
import { AboutComponent } from './views/about/about.component';
import { ContactComponent } from './views/contact/contact.component';
import { KaufenComponent } from './views/kaufen/kaufen.component';

@NgModule({
    declarations: [
        AppComponent,
        ThreejsComponent,
        SpinningWheelComponent,
        HomeComponent,
        InfoTextComponent,
        ProductNewsComponent,
        ProductNewsPopUpComponent,
        InvalidEmailSnackBarComponent,
        InfoArrowComponent,
        FirstSideComponent,
        SecondSideComponent,
        LongInfoTextComponent,
        AboutComponent,
        ContactComponent,
        KaufenComponent,
    ],
    imports: [
        BrowserAnimationsModule,
        BrowserModule,
        AppRoutingModule,
        MaterialModule,
        FormsModule,
        ReactiveFormsModule,
    ],
    providers: [ ],
    bootstrap: [AppComponent],
})
export class AppModule {}
