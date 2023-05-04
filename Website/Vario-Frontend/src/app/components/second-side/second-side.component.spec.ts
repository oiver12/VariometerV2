import { ComponentFixture, TestBed } from '@angular/core/testing';

import { SecondSideComponent } from './second-side.component';

describe('SecondSideComponent', () => {
  let component: SecondSideComponent;
  let fixture: ComponentFixture<SecondSideComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ SecondSideComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(SecondSideComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
