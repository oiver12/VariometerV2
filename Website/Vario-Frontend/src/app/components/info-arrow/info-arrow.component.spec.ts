import { ComponentFixture, TestBed } from '@angular/core/testing';

import { InfoArrowComponent } from './info-arrow.component';

describe('InfoArrowComponent', () => {
  let component: InfoArrowComponent;
  let fixture: ComponentFixture<InfoArrowComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ InfoArrowComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(InfoArrowComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
