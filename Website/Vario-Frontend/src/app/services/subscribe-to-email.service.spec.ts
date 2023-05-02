import { TestBed } from '@angular/core/testing';

import { SubscribeToEmailService } from './subscribe-to-email.service';

describe('SubscribeToEmailService', () => {
  let service: SubscribeToEmailService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(SubscribeToEmailService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
