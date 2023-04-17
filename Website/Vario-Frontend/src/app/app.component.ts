import { Component } from '@angular/core';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {
  title = 'Vario-Frontend';
  darkTheme = true;
  ngOnInit(): void {
    if (this.darkTheme) {
      document.body.classList.add('dark-theme');
    }
  }
  toggleDarkTheme(): void {
    this.darkTheme = !this.darkTheme;
    document.body.classList.toggle('dark-theme');
  }
  goToShop(): void {
  }
}
