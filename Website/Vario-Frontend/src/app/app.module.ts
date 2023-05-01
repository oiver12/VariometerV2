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
    ],
    imports: [
        BrowserAnimationsModule,
        BrowserModule,
        AppRoutingModule,
        MaterialModule,
        FormsModule,
        ReactiveFormsModule,
    ],
    providers: [],
    bootstrap: [AppComponent],
})
export class AppModule {}
