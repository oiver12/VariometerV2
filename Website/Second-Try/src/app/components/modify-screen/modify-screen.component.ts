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
    startPosition: {
        left: number;
        right: number;
        top: number;
        bottom: number;
    };
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
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Barometer Altitude',
            unit: 'None',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Thermik Assistant',
            unit: 'None',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'SD Card use',
            unit: 'None',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
    ];
    buttonTitleText = [
        {
            title: 'Ground Speed',
            unit: 'km/h',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Active Vario',
            unit: 'm/s',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Lazy Vario',
            unit: 'm/s',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Vertical Acceleration',
            unit: 'm/s2',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Barometer Altitude',
            unit: 'm',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'GPS Altitude',
            unit: 'm',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Above Ground Level',
            unit: 'm',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Time',
            unit: '',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
            position: { left: 0, right: 0, top: 0, bottom: 0 },
        },
        {
            title: 'Pressure',
            unit: 'hPa',
            startPosition: { left: 0, right: 0, top: 0, bottom: 0 },
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
            this.copyChosenElements[
                this.copyChosenElements.length - 1
            ].startPosition = this.placeNewElement();
            this.copyChosenElements[
                this.copyChosenElements.length - 1
            ].position = this.placeNewElement();
            this.chosenElements = this.copyChosenElements;
        }, 0);
    }
    placeNewElement(): Element['startPosition'] {
        // we bruteforce the new position of the element
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
        const stepsDown = 8;
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
                        top: (counterDown * height) / 2,
                        bottom: (counterDown * height) / 2 + height,
                    } as Rect;
                    if (
                        !this.elementHasIntersection(
                            elementPosition,
                            this.chosenElements.length - 1
                        )
                    ) {
                        return elementPosition;
                    }
                }
            }
        }
        return { left: 0, right: width, top: 0, bottom: height };
    }
    elementHasIntersection(
        element: Element['startPosition'],
        id: number
    ): boolean {
        if (!this.isCompletelyWithinBoundary(element)) {
            return true;
        }
        let hasIntersection = false;
        this.chosenElements.forEach((element2, index) => {
            if (index !== id) {
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
                left: this.elementPositionX - this.boundaryElement.left,
                right:
                    this.elementPositionX +
                    dimensions.width -
                    this.boundaryElement.left,
                top: this.elementPositionY - this.boundaryElement.top,
                bottom:
                    this.elementPositionY +
                    dimensions.height -
                    this.boundaryElement.top,
            } as Rect;
            // the wanted bahaviour is the following:
            // a element can't be dragged over another element but should snap to the nearest free position

            this.chosenElements.forEach((element, index) => {
                if (id !== index) {
                    if (
                        this.haveIntersection(elementPosition, element.position)
                    ) {
                        console.log('intersection');
                        // snap to nearest free position
                        // for this look which snap position is the nearest
                        // if this position is free, look if it is possible to snap to this position
                        // if not, look for the next nearest position
                        const newPosition = this.snapToNearestFreePosition(
                            elementPosition,
                            element.position,
                            id
                        );
                        this.elementPositionX =
                            newPosition.left + this.boundaryElement.left;
                        this.elementPositionY =
                            newPosition.top + this.boundaryElement.top;
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
    isCompletelyWithinBoundary(r1: Rect): boolean {
        return (
            r1.top >= 0 &&
            r1.left >= 0 &&
            r1.bottom <= this.boundaryElement.height &&
            r1.right <= this.boundaryElement.width
        );
    }

    snapToNearestFreePosition(
        elementPosition: Rect,
        otherPosition: Rect,
        id: number
    ): Rect {
        const snapPosition = { ...elementPosition };

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
        // If a position is not free, we put 1000 in the distances array and repeat the process
        for (let i = 0; i < distances.length; i++) {
            switch (minDistance) {
                case distances[0]:
                    snapPosition.left =
                        otherPosition.left -
                        (elementPosition.right - elementPosition.left);
                    snapPosition.right = otherPosition.left;
                    if (!this.elementHasIntersection(snapPosition, id)) {
                        return snapPosition;
                    } else {
                        distances[0] = 1000;
                    }
                    break;
                case distances[1]:
                    snapPosition.left = otherPosition.right;
                    snapPosition.right =
                        otherPosition.right +
                        (elementPosition.right - elementPosition.left);
                    if (!this.elementHasIntersection(snapPosition, id)) {
                        return snapPosition;
                    } else {
                        distances[1] = 1000;
                    }
                    break;
                case distances[2]:
                    snapPosition.top =
                        otherPosition.top -
                        (elementPosition.bottom - elementPosition.top);
                    snapPosition.bottom = otherPosition.top;
                    if (!this.elementHasIntersection(snapPosition, id)) {
                        return snapPosition;
                    } else {
                        distances[2] = 1000;
                    }
                    break;
                default:
                    snapPosition.top = otherPosition.bottom;
                    snapPosition.bottom =
                        otherPosition.bottom +
                        (elementPosition.bottom - elementPosition.top);
                    if (!this.elementHasIntersection(snapPosition, id)) {
                        return snapPosition;
                    } else {
                        distances[3] = 1000;
                    }
                    break;
            }
        }
        return snapPosition;
    }
    calculatePosition(element: HTMLElement): Rect {
        const rect = element.getBoundingClientRect();
        return {
            left: rect.left - this.boundaryElement.left,
            right: rect.left + rect.width - this.boundaryElement.left,
            top: rect.top - this.boundaryElement.top,
            bottom: rect.top + rect.height - this.boundaryElement.top,
        };
    }
    onMouseUp() {
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
