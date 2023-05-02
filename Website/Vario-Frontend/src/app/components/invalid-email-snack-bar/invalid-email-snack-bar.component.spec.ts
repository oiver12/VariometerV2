import { ComponentFixture, TestBed } from '@angular/core/testing';

import { InvalidEmailSnackBarComponent } from './invalid-email-snack-bar.component';

describe('InvalidEmailSnackBarComponent', () => {
  let component: InvalidEmailSnackBarComponent;
  let fixture: ComponentFixture<InvalidEmailSnackBarComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ InvalidEmailSnackBarComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(InvalidEmailSnackBarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
