import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SpinningWheelComponent } from './components/spinning-wheel/spinning-wheel.component';
import { HomeComponent } from './views/home/home.component';

const routes: Routes = [
  { path: '', redirectTo: '/home', pathMatch: 'full' },
  { path: 'spinn', component: SpinningWheelComponent},
  { path: 'home', component: HomeComponent}
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { 

}
