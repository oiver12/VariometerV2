import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MatDividerModule } from '@angular/material/divider'; 
import { MatIconModule } from '@angular/material/icon';
import { MatButtonModule } from '@angular/material/button'; 

const components = [
  MatDividerModule,
  MatIconModule,
  MatButtonModule
];

@NgModule({
  declarations: [
  ],
  imports: [
    CommonModule,
    ...components
  ],
  exports: components,
})

export class MaterialModule { }
