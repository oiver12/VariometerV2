import { Component, ViewChild, Input, ElementRef } from '@angular/core';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';
import { transition, style, animate, trigger } from '@angular/animations';
import { HostListener } from '@angular/core';

const leaveTrans = transition(':leave', [
    style({
        opacity: 1,
    }),
    animate(
        '0.5s ease-out',
        style({
            opacity: 0,
        })
    ),
]);

const fadeOut = trigger('fadeOut', [leaveTrans]);

@Component({
    selector: 'app-three-js',
    templateUrl: './three-js.component.html',
    styleUrls: ['./three-js.component.css'],
    animations: [fadeOut],
})
export class ThreeJsComponent {
    @Input() name!: string;
    @ViewChild('canvas') canvasRef!: ElementRef;

    renderer = new THREE.WebGLRenderer();
    scene: THREE.Scene;
    camera: THREE.PerspectiveCamera;
    controls!: OrbitControls;
    mesh: THREE.Mesh | undefined;
    light: THREE.PointLight | undefined;
    hintVisible = true;

    // add a event listener for window resize
    // and update the camera aspect ratio accordingly
    // to prevent the scene from stretching
    @HostListener('window:resize')
    onResize() {
        this.camera.aspect = this.calculateAspectRatio();
        this.camera.updateProjectionMatrix();
        if (document.getElementsByClassName('three-js')![0]) {
            this.renderer.setSize(
                document.getElementsByClassName('three-js')![0].clientWidth,
                document.getElementsByClassName('three-js')![0].clientHeight
            );
        }
    }

    private calculateAspectRatio(): number {
        const height = this.canvas.clientHeight;
        if (height === 0) {
            return 0;
        }
        return this.canvas.clientWidth / this.canvas.clientHeight;
    }

    private get canvas(): HTMLCanvasElement {
        return this.canvasRef.nativeElement;
    }

    constructor() {
        this.scene = new THREE.Scene();
        this.camera = new THREE.PerspectiveCamera(35, 800 / 640, 0.1, 1000);
    }

    ngAfterViewInit() {
        this.configCamera();
        this.configRenderer();
        this.configControls();

        this.createLight();
        this.createMesh();

        this.animate();
        this.canvas.addEventListener('mousedown', () => {
            this.hintVisible = false;
        });
    }

    configCamera() {
        this.camera.aspect = this.calculateAspectRatio();
        this.camera.updateProjectionMatrix();
        this.camera.position.set(-17, 9, 17);
        this.camera.lookAt(this.scene.position);
    }

    configRenderer() {
        this.renderer = new THREE.WebGLRenderer({
            canvas: this.canvas,
            antialias: true,
            alpha: true,
        });
        this.renderer.setPixelRatio(devicePixelRatio);
        // setClearColor for transparent background
        // i.e. scene or canvas background shows through
        this.renderer.setClearColor(0x000000, 0);
        this.renderer.setSize(
            this.canvas.clientWidth,
            this.canvas.clientHeight
        );
    }

    configControls() {
        this.controls = new OrbitControls(this.camera, this.canvas);
        this.controls.autoRotate = true;
        this.controls.autoRotateSpeed = 0.5;
        this.controls.enableDamping = true;
        this.controls.enableZoom = false;
        this.controls.update();
    }

    createLight() {
        this.light = new THREE.PointLight(0xffffff);
        this.light.position.set(-15, 10, 15);
        this.scene.add(this.light);
    }

    createMesh() {
        const loader = new GLTFLoader();
        loader.load('../../assets/models/test1.gltf', (gltf) => {
            this.mesh = gltf.scene.children[0] as THREE.Mesh;
            this.scene.add(this.mesh);
        });
    }

    animate() {
        window.requestAnimationFrame(() => this.animate());
        this.controls.update();
        this.renderer.render(this.scene, this.camera);
    }
}
