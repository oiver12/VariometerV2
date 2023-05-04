import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { LoginComponent } from './views/login/login.component';
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
import { BottomInformationComponent } from './components/bottom-information/bottom-information.component';

@NgModule({
    declarations: [
        AppComponent,
        LoginComponent,
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
        BottomInformationComponent,
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
