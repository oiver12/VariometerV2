import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { MaterialModule } from './material/material.module';
import { HomeComponent } from './views/home/home.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ThreeJsComponent } from './components/three-js/three-js.component';
import { HomeTopComponent } from './components/home/home-top/home-top.component';
import { HomeMiddleComponent } from './components/home/home-middle/home-middle.component';
import { HomeBottomComponent } from './components/home/home-bottom/home-bottom.component';
import { CardComponent } from './components/card/card.component';
import { ParallaxDirective } from './components/parallax/parallax.directive';
import { BulletListComponent } from './components/bullet-list/bullet-list.component';
import { ModifySoundsComponent } from './components/modify-sounds/modify-sounds.component';
import { ModifyComponent } from './views/modify/modify.component';
import { ModifyScreenComponent } from './components/modify-screen/modify-screen.component';
import {
    LineSeriesService,
    ColumnSeriesService,
    CategoryService,
    DataEditingService,
    TooltipService,
    LegendService,
    ChartModule,
} from '@syncfusion/ej2-angular-charts';

@NgModule({
    declarations: [
        AppComponent,
        HomeComponent,
        ThreeJsComponent,
        HomeTopComponent,
        HomeMiddleComponent,
        HomeBottomComponent,
        CardComponent,
        ParallaxDirective,
        BulletListComponent,
        ModifySoundsComponent,
        ModifyComponent,
    ],
    imports: [
        BrowserModule,
        AppRoutingModule,
        MaterialModule,
        BrowserAnimationsModule,
        ModifyScreenComponent,
        ChartModule,
    ],
    providers: [
        LegendService,
        LineSeriesService,
        ColumnSeriesService,
        CategoryService,
        DataEditingService,
        TooltipService,
    ],
    bootstrap: [AppComponent],
})
export class AppModule {}
