import { ComponentFixture, TestBed } from '@angular/core/testing';

import { HomeMiddleComponent } from './home-middle.component';

describe('HomeMiddleComponent', () => {
  let component: HomeMiddleComponent;
  let fixture: ComponentFixture<HomeMiddleComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ HomeMiddleComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(HomeMiddleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
