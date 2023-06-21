import { Component } from '@angular/core';
import { CdkDrag, DragRef, Point } from '@angular/cdk/drag-drop';
import { CommonModule } from '@angular/common';
import { MaterialModule } from 'app/material/material.module';

type Rect = {
    top: number;
    right: number;
    bottom: number;
    left: number;
    x?: number;
    y?: number;
    width?: number;
    height?: number;
};
type Element = {
    title: string;
    unit: string;
    position: {
        left: number;
        right: number;
        top: number;
        bottom: number;
    };
};
@Component({
    selector: 'app-modify-screen',
    templateUrl: './modify-screen.component.html',
    styleUrls: ['./modify-screen.component.css'],
    standalone: true,
    imports: [CommonModule, CdkDrag, MaterialModule],
})
export class ModifyScreenComponent {
    buttonTitleGraph = [
        {
            title: 'Kompass',
            unit: 'None',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Barometer Altitude',
            unit: 'None',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Thermik Assistant',
            unit: 'None',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'SD Card use',
            unit: 'None',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
    ];
    buttonTitleText = [
        {
            title: 'Ground Speed',
            unit: 'km/h',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Active Vario',
            unit: 'm/s',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Lazy Vario',
            unit: 'm/s',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Vertical Acceleration',
            unit: 'm/s2',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Barometer Altitude',
            unit: 'm',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'GPS Altitude',
            unit: 'm',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Above Ground Level',
            unit: 'm',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Time',
            unit: '',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Pressure',
            unit: 'hPa',
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
    ];
    chosenElements: Element[] = [];
    copyChosenElements: Element[] = [];
    selectedTitle: Element[] = this.buttonTitleText;
    elementPositionX: number = 0;
    elementPositionY: number = 0;
    boundaryElement!: {
        x: number;
        y: number;
        width: number;
        height: number;
        top: number;
        right: number;
        bottom: number;
        left: number;
    };

    ngAfterViewInit() {
        this.boundaryElement = document
            .getElementsByClassName('boundary')[0]
            .getBoundingClientRect();
    }

    changeSelectedTitle() {
        this.selectedTitle =
            this.selectedTitle === this.buttonTitleText
                ? this.buttonTitleGraph
                : this.buttonTitleText;
    }

    createBox(element: Element) {
        this.chosenElements.push(element);
        this.copyChosenElements = structuredClone(this.chosenElements);
        // to get the Element rendered
        setTimeout(() => {
            const elements = Array.from(
                document.getElementsByClassName('screen-element')
            ) as HTMLElement[];

            this.chosenElements = this.copyChosenElements;
            this.copyChosenElements[
                this.copyChosenElements.length - 1
            ].position = this.placeNewElement();
            console.log(this.copyChosenElements);
            elements[elements.length - 1].style.left =
                this.copyChosenElements[this.copyChosenElements.length - 1]
                    .position.left + 'px';
            elements[elements.length - 1].style.top =
                this.copyChosenElements[this.copyChosenElements.length - 1]
                    .position.top + 'px';
            console.log(elements);
        }, 0);
        setTimeout(() => {
            // this.chosenElements = this.copyChosenElements;
        }, 10000);
    }
    placeNewElement(): Element['position'] {
        // to set the position of the element we do it "ugly"
        // we try to place the element at the top left corner of the screen
        // if there is a intersection with another element or the boundary, we move the element to the right
        // we move it in steps of element.width / 5
        // if after 15 steps there is still a intersection (all the way to the right), we move the element to the start
        // then we move the element in steps of element.height / 5 down for 5 steps
        // if there is still a intersection, we move the element to the right for 15 steps
        // if there is still a intersection, we move the element all the way to the left
        // then we move the element in steps of element.height / 5 down for 5 steps
        // etc.
        // we do this 4 times, then we give up
        const stepsLeft = 15;
        const stepsDown = 5;
        const counterMax = 4;
        const documentElement = document.getElementById(
            (this.chosenElements.length - 1).toString()
        );
        const width = documentElement!.getBoundingClientRect().width;
        const height = documentElement!.getBoundingClientRect().height;
        let elementPosition = {
            left: 0,
            right: width,
            top: 0,
            bottom: height,
        } as Rect;
        for (let counter = 0; counter < counterMax; counter++) {
            for (let counterDown = 0; counterDown < stepsDown; counterDown++) {
                for (
                    let counterLeft = 0;
                    counterLeft < stepsLeft;
                    counterLeft++
                ) {
                    elementPosition = {
                        left: (counterLeft * width) / 5,
                        right: (counterLeft * width) / 5 + width,
                        top: (counterDown * height) / 5,
                        bottom: (counterDown * height) / 5 + height,
                    } as Rect;
                    if (!this.elementHasIntersection(elementPosition)) {
                        return elementPosition;
                    }
                }
            }
        }
        return elementPosition;
    }
    elementHasIntersection(element: Element['position']): boolean {
        if (this.haveIntersectionWithBoundary(element)) {
            return true;
        }
        let hasIntersection = false;
        this.chosenElements.forEach((element2, index) => {
            if (index !== this.chosenElements.length - 1) {
                if (this.haveIntersection(element, element2.position)) {
                    hasIntersection = true;
                    return;
                }
            }
        });
        return hasIntersection;
    }
    snap(
        id: number
    ): (
        userPointerPosition: Point,
        dragRef: DragRef<any>,
        dimensions: { width: number; height: number },
        pickupPositionInElement: Point
    ) => Point {
        return (
            userPointerPosition,
            dragRef,
            dimensions,
            pickupPositionInElement
        ) => {
            this.elementPositionX =
                userPointerPosition.x - pickupPositionInElement.x;
            this.elementPositionY =
                userPointerPosition.y - pickupPositionInElement.y;

            // keep element in boundary
            this.elementPositionX = Math.max(
                this.boundaryElement.left,
                Math.min(
                    this.elementPositionX,
                    this.boundaryElement.right - dimensions.width
                )
            );
            this.elementPositionY = Math.max(
                this.boundaryElement.top,
                Math.min(
                    this.elementPositionY,
                    this.boundaryElement.bottom - dimensions.height
                )
            );
            const elementPosition = {
                left: this.elementPositionX,
                right: this.elementPositionX + dimensions.width,
                top: this.elementPositionY,
                bottom: this.elementPositionY + dimensions.height,
            } as Rect;
            // the wanted bahaviour is the following:
            // a element can't be dragged over another element but should snap to the nearest free position

            this.chosenElements.forEach((element, index) => {
                // this.calculatePosition(document.getElementById(id.toString())!);
                if (id !== index) {
                    if (
                        this.haveIntersection(elementPosition, element.position)
                    ) {
                        // snap to nearest free position
                        // for this look which snap position is the nearest
                        // if this position is free, look if it is possible to snap to this position
                        // if not, look for the next nearest position
                        const newPosition = this.snapToNearestFreePosition(
                            elementPosition,
                            element.position
                        );
                        this.elementPositionX = newPosition.left;
                        this.elementPositionY = newPosition.top;
                    }
                }
            });

            return {
                x: this.elementPositionX,
                y: this.elementPositionY,
            };
        };
    }
    haveIntersection(r1: Rect, r2: Rect): boolean {
        return (
            r1.bottom > r2.top &&
            r1.right > r2.left &&
            r1.top < r2.bottom &&
            r1.left < r2.right
        );
    }
    haveIntersectionWithBoundary(r1: Rect): boolean {
        return (
            r1.bottom > this.boundaryElement.top &&
            r1.right > this.boundaryElement.left &&
            r1.top < this.boundaryElement.bottom &&
            r1.left < this.boundaryElement.right
        );
    }
    snapToNearestFreePosition(
        elementPosition: Rect,
        otherPosition: Rect
    ): Rect {
        const snapPosition = { ...elementPosition }; // Clone the element position

        // Calculate the distances from the element center to the other rectangle edges
        const distances: number[] = [
            Math.abs(otherPosition.left - elementPosition.right),
            Math.abs(otherPosition.right - elementPosition.left),
            Math.abs(otherPosition.top - elementPosition.bottom),
            Math.abs(otherPosition.bottom - elementPosition.top),
        ];

        // Find the minimum distance
        const minDistance = Math.min(...distances);

        // Snap the element to the nearest free position based on the minimum distance
        if (minDistance === distances[0]) {
            snapPosition.left =
                otherPosition.left -
                (elementPosition.right - elementPosition.left);
            snapPosition.right = otherPosition.left;
        } else if (minDistance === distances[1]) {
            snapPosition.left = otherPosition.right;
            snapPosition.right =
                otherPosition.right +
                (elementPosition.right - elementPosition.left);
        } else if (minDistance === distances[2]) {
            snapPosition.top =
                otherPosition.top -
                (elementPosition.bottom - elementPosition.top);
            snapPosition.bottom = otherPosition.top;
        } else {
            snapPosition.top = otherPosition.bottom;
            snapPosition.bottom =
                otherPosition.bottom +
                (elementPosition.bottom - elementPosition.top);
        }

        return snapPosition;
    }
    calculatePosition(element: HTMLElement): Rect {
        const rect = element.getBoundingClientRect();
        return {
            left: rect.left,
            right: rect.left + rect.width,
            top: rect.top,
            bottom: rect.top + rect.height,
        };
    }
    onMouseUp() {
        // calculate the new position of all elements
        this.chosenElements.forEach((element, index) => {
            const elementPosition = this.calculatePosition(
                document.getElementById(index.toString())!
            );
            element.position.left = elementPosition.left;
            element.position.right = elementPosition.right;
            element.position.top = elementPosition.top;
            element.position.bottom = elementPosition.bottom;
        });
    }
}
