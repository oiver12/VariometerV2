import { ComponentFixture, TestBed } from '@angular/core/testing';

import { FirstSideComponent } from './first-side.component';

describe('FirstSideComponent', () => {
  let component: FirstSideComponent;
  let fixture: ComponentFixture<FirstSideComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ FirstSideComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(FirstSideComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
