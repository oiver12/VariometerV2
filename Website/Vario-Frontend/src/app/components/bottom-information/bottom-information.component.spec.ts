import { ComponentFixture, TestBed } from '@angular/core/testing';

import { BottomInformationComponent } from './bottom-information.component';

describe('BottomInformationComponent', () => {
  let component: BottomInformationComponent;
  let fixture: ComponentFixture<BottomInformationComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ BottomInformationComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(BottomInformationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
