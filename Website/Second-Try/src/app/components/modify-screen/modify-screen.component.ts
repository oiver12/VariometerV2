import { Component, Renderer2, ElementRef, HostListener } from '@angular/core';
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
    // we use this array to store the last known valid positions of each element from the chosenElements array
    oldValidPositions: Rect[] = [];
    toDelete: [boolean, number] = [false, 0];
    oldScale: number = 0;
    counter: number = 0; // this is a counter which keeps track of our current position of the different screens

    ngAfterViewInit() {
        this.boundaryElement = document
            .getElementsByClassName('boundary')[0]
            .getBoundingClientRect();
        this.oldScale = this.boundaryElement.width;
    }

    @HostListener('window:resize', ['$event'])
    onResize(event: any) {
        this.boundaryElement = document
            .getElementsByClassName('boundary')[0]
            .getBoundingClientRect();
        console.log('resize');
        // we need to recalculate all the positions of the elements in the chosenElements array

        if (this.oldScale !== window.innerWidth && this.oldScale !== 0) {
            const scaleVariable = this.boundaryElement.width / this.oldScale;
            this.chosenElements.forEach((element, index) => {
                element.position.left *= scaleVariable;
                element.position.right *= scaleVariable;
                element.position.top *= scaleVariable;
                element.position.bottom *= scaleVariable;
                element.startPosition.left *= scaleVariable;
                element.startPosition.right *= scaleVariable;
                element.startPosition.top *= scaleVariable;
                element.startPosition.bottom *= scaleVariable;
            });
            const screenElements = document.getElementsByClassName(
                'screen-element'
            ) as HTMLCollectionOf<HTMLElement>;
            for (let i = 0; i < screenElements.length; i++) {
                const element = screenElements[i];
                const regex = /translate3d\((-?[\d.]+)px,\s*(-?[\d.]+)px,.*\)/;

                if (element.style.transform.match(regex)) {
                    const translationX =
                        parseFloat(element.style.transform.match(regex)![1]) *
                        (scaleVariable - 1);
                    const translationY =
                        parseFloat(element.style.transform.match(regex)![2]) *
                        (scaleVariable - 1);
                    this.chosenElements[i].startPosition.left += translationX;
                    this.chosenElements[i].startPosition.right += translationX;
                    this.chosenElements[i].startPosition.top += translationY;
                    this.chosenElements[i].startPosition.bottom += translationY;
                }
            }
            this.oldScale = this.boundaryElement.width;
        }
    }
    changeSelectedTitle() {
        this.selectedTitle =
            this.selectedTitle === this.buttonTitleText
                ? this.buttonTitleGraph
                : this.buttonTitleText;
    }
    createBox(element: Element) {
        this.chosenElements.push(element);
        setTimeout(() => {
            this.chosenElements[this.chosenElements.length - 1].startPosition =
                this.placeNewElement();
            this.chosenElements[this.chosenElements.length - 1].position =
                this.placeNewElement();
        }, 0);
    }
    placeNewElement(): Element['startPosition'] {
        // we bruteforce the new position of the element
        // we try to place the element at the top left corner of the screen
        // if there is a> intersection with another element or the boundary, we move the element to the right
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
        const {width, height} = documentElement!.getBoundingClientRect();
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
        this.oldValidPositions.push(elementPosition);
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
            if (index !== id && this.haveIntersection(element, element2.position)) {
                  hasIntersection = true;
                  return;
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
            console.log(
                document.getElementById(id.toString())!.style.transform
            );
            console.log(userPointerPosition, pickupPositionInElement);
            this.elementPositionX =
                userPointerPosition.x - pickupPositionInElement.x;
            this.elementPositionY =
                userPointerPosition.y - pickupPositionInElement.y;

            // a user can delete a element by dragging it out of the boundary with a tolerance of 50% of the element width
            // Calculate the tolerance for snapping (50% of element width)
            const snapToleranceX = dimensions.width * 0.75;
            const snapToleranceY = dimensions.height * 0.75;

            // Snap if the element is dragged out of the boundary with tolerance
            if (
                this.elementPositionX <
                this.boundaryElement.left - snapToleranceX
            ) {
                this.toDelete = [true, id];
            } else if (
                this.elementPositionX >
                this.boundaryElement.right - dimensions.width + snapToleranceX
            ) {
                this.toDelete = [true, id];
            } else if (
                this.elementPositionY <
                this.boundaryElement.top - snapToleranceY
            ) {
                this.toDelete = [true, id];
            } else if (
                this.elementPositionY >
                this.boundaryElement.bottom - dimensions.height + snapToleranceY
            ) {
                this.toDelete = [true, id];
            } else if (this.toDelete[0] && this.toDelete[1] === id) {
                this.toDelete = [false, 0];
            }

            if (this.toDelete[0]) {
                return {
                    x: this.elementPositionX,
                    y: this.elementPositionY,
                };
            }
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
                if (id !== index && this.haveIntersection(elementPosition, element.position)) {
                      const newPosition = this.snapToNearestFreePosition(
                          elementPosition,
                          element.position,
                          id,
                          0
                      );
                      this.elementPositionX =
                          newPosition.left + this.boundaryElement.left;
                      this.elementPositionY =
                          newPosition.top + this.boundaryElement.top;
                }
            });
            this.oldValidPositions[id] = {
                left: this.elementPositionX - this.boundaryElement.left,
                right: this.elementPositionX + dimensions.width,
                top: this.elementPositionY - this.boundaryElement.top,
                bottom: this.elementPositionY + dimensions.height,
            } as Rect;
            console.log(dragRef);
            return {
                x: this.elementPositionX,
                y: this.elementPositionY,
            };
        };
    }
    haveIntersection(r1: Rect, r2: Rect): boolean {
        // To adjust for that It also fits if there is not enough space for a rect but only for 1 px we have a buffer
        const buffer = 2;
        return (
            r1.bottom - buffer > r2.top &&
            r1.right - buffer > r2.left &&
            r1.top + buffer < r2.bottom &&
            r1.left + buffer < r2.right
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
        id: number,
        counter: number,
        oldDistances?: number[]
    ): Rect {
        const snapPosition = { ...elementPosition };
        var distances: number[] = [];
        // Calculate the distances from the element center to the other rectangle edges
        if (!oldDistances) {
            distances = [
                Math.abs(otherPosition.left - elementPosition.right),
                Math.abs(otherPosition.right - elementPosition.left),
                Math.abs(otherPosition.top - elementPosition.bottom),
                Math.abs(otherPosition.bottom - elementPosition.top),
            ];
        } else {
            distances = oldDistances;
        }

        // Find the minimum distance
        const minDistance = Math.min(...distances);

        // Calculate the snap positions for each case
        const snapPositions = [
            {
                left:
                    otherPosition.left -
                    (elementPosition.right - elementPosition.left),
                right: otherPosition.left,
                top: snapPosition.top,
                bottom: snapPosition.bottom,
            },
            {
                left: otherPosition.right,
                right:
                    otherPosition.right +
                    (elementPosition.right - elementPosition.left),
                top: snapPosition.top,
                bottom: snapPosition.bottom,
            },
            {
                left: snapPosition.left,
                right: snapPosition.right,
                top:
                    otherPosition.top -
                    (elementPosition.bottom - elementPosition.top),
                bottom: otherPosition.top,
            },
            {
                left: snapPosition.left,
                right: snapPosition.right,
                top: otherPosition.bottom,
                bottom:
                    otherPosition.bottom +
                    (elementPosition.bottom - elementPosition.top),
            },
        ];

        // Snap the element to the nearest free position based on the minimum distance
        const minDistanceIndex = distances.indexOf(minDistance);
        const snapPos = snapPositions[minDistanceIndex];
        if (!this.elementHasIntersection(snapPos, id)) {
            return snapPos;
        }
        if (
            distances[0] === Number.MAX_VALUE &&
            distances[1] === Number.MAX_VALUE &&
            distances[2] === Number.MAX_VALUE &&
            distances[3] === Number.MAX_VALUE
        ) {
            return this.oldValidPositions[id];
        }
        // If no suitable position is found and the counter is smaller than 1, try recursion
        if (counter < 1) {
            let recursiveResult: Rect | undefined;
            this.chosenElements.forEach((element, index) => {
                if (
                    id !== index &&
                    this.haveIntersection(snapPos, element.position)
                ) {
                    recursiveResult = this.snapToNearestFreePosition(
                        snapPos,
                        element.position,
                        id,
                        1
                    );
                }
            });
            if (recursiveResult && !this.elementHasIntersection(recursiveResult, id)) {
                  return recursiveResult;
            }
        }
        if (counter === 1) {
            // If no suitable position is found and the counter is 1, try recursion with the next nearest position
            distances[minDistanceIndex] = Number.MAX_VALUE;
            return this.snapToNearestFreePosition(
                elementPosition,
                otherPosition,
                id,
                0,
                distances
            );
        }
        if (!this.elementHasIntersection(snapPos, id)) {
            return snapPosition;
        } else {
            return this.oldValidPositions[id];
        }
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
        if (this.toDelete[0]) {
            this.chosenElements.splice(this.toDelete[1], 1);
            this.toDelete = [false, 0];
        }
    }
    goBackward() {
        if (this.counter > 0) {
            this.counter--;
            return;
        }
        this.counter = 3;
    }
    goForward() {
        if (this.counter < 3) {
            this.counter++;
            return;
        }
        this.counter = 0;
    }
}
