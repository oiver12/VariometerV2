import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { HomeComponent } from './views/home/home.component';
import { ModifyComponent } from './views/modify/modify.component';

const routes: Routes = [
    { path: 'home', component: HomeComponent },
    { path: 'modify', component: ModifyComponent},
    { path: '**', redirectTo: 'home' },
];

@NgModule({
    imports: [RouterModule.forRoot(routes)],
    exports: [RouterModule],
})
export class AppRoutingModule {}
