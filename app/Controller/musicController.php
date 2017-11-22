<?php

App::uses('AppController', 'Controller');

class MusicController extends AppController {
    public function beforeFilter() {
        parent::beforeFilter();
        //$this->Auth->allow();
    }

    public function index() {

      exit();
    }
    
    public function note_edit($id = 0){
        
        $this->layout = '';
    }
    
}
