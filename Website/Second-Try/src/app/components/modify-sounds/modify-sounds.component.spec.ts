import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ModifySoundsComponent } from './modify-sounds.component';

describe('ModifySoundsComponent', () => {
  let component: ModifySoundsComponent;
  let fixture: ComponentFixture<ModifySoundsComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ ModifySoundsComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(ModifySoundsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
