import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Injectable({
  providedIn: 'root'
})
export class SubscribeToEmailService {

  constructor() { }
  sendEmail(email: string) {
    // this.http.post('https://us-central1-ecommerce-website-8f9b6.cloudfunctions.net/subscribeToEmail', { email }).subscribe();
  }
}
