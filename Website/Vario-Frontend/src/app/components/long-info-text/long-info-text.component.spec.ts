import { ComponentFixture, TestBed } from '@angular/core/testing';

import { LongInfoTextComponent } from './long-info-text.component';

describe('LongInfoTextComponent', () => {
  let component: LongInfoTextComponent;
  let fixture: ComponentFixture<LongInfoTextComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ LongInfoTextComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(LongInfoTextComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
