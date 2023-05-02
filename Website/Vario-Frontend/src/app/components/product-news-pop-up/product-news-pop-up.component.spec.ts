import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ProductNewsPopUpComponent } from './product-news-pop-up.component';

describe('ProductNewsPopUpComponent', () => {
  let component: ProductNewsPopUpComponent;
  let fixture: ComponentFixture<ProductNewsPopUpComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ ProductNewsPopUpComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(ProductNewsPopUpComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
