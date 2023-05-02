import { Component, ViewChild, Input, ElementRef } from '@angular/core';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';
import { transition, style, animate, trigger } from '@angular/animations';

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
    selector: 'app-threejs',
    templateUrl: './threejs.component.html',
    styleUrls: ['./threejs.component.css'],
    animations: [
        fadeOut
    ]
})
export class ThreejsComponent {
    @Input() name!: string;
    @ViewChild('canvas') canvasRef!: ElementRef;

    renderer = new THREE.WebGLRenderer();
    scene: THREE.Scene;
    camera: THREE.PerspectiveCamera;
    controls!: OrbitControls;
    mesh: THREE.Mesh | undefined;
    light: THREE.PointLight | undefined;
    hintVisible = true;

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
        this.configScene();
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

    configScene() {
        const background = Number(
            '0x'.concat(
                getComputedStyle(document.body)
                    .getPropertyValue('--left-bkg-color')
                    .slice(1)
            )
        );
        this.scene.background = new THREE.Color(background);
    }

    configCamera() {
        this.camera.aspect = this.calculateAspectRatio();
        this.camera.updateProjectionMatrix();
        this.camera.position.set(-13, 9, 13);
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
        this.light.position.set(-10, 10, 10);
        this.scene.add(this.light);
    }

    createMesh() {
        const loader = new GLTFLoader();
        loader.load('../../assets/models/test.gltf', (gltf) => {
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
