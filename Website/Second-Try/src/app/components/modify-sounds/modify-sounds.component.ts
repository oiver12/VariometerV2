import { Component } from '@angular/core';

@Component({
    selector: 'app-modify-sounds',
    templateUrl: './modify-sounds.component.html',
    styleUrls: ['./modify-sounds.component.css'],
})
export class ModifySoundsComponent {
    public primaryXAxis?: Object;
    public columnData?: Object[];
    public lineData?: Object[];
    public title?: string;
    public primaryYAxis?: Object;
    public chartArea?: Object;
    public marker?: Object;
    public dragSettings?: Object;
    public tooltip?: Object;
    ngOnInit(): void {
        this.lineData = [
            { x: '2005', y: 21 },
            { x: '2006', y: 22 },
            { x: '2007', y: 36 },
            { x: '2008', y: 34 },
            { x: '2009', y: 54 },
            { x: '2010', y: 55 },
            { x: '2011', y: 60 },
        ];
        this.primaryXAxis = {
            valueType: 'Category',
            minimum: -0.5,
            maximum: 6.5,
            labelPlacement: 'OnTicks',
            majorGridLines: { width: 0 },
        };
        this.primaryYAxis = {
            rangePadding: 'None',
            minimum: 0,
            title: 'Sales',
            labelFormat: '{value}%',
            maximum: 100,
            interval: 20,
            lineStyle: { width: 0 },
            majorTickLines: { width: 0 },
            minorTickLines: { width: 0 },
        };
        this.chartArea = {
            border: {
                width: 0,
            },
        };
        this.title = 'Inflation - Consumer Price';
        this.marker = {
            visible: true,
            width: 10,
            height: 10,
        };
        this.dragSettings = {
            enable: true,
        };
        this.tooltip = {
            enable: true,
        };
    }
}
