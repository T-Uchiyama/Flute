<?php
    class Music extends AppModel
    {
        public $actsAs = array(
            'Upload.Upload' => array(
                'image' => array(
                    'fields' => array(
                        'dir' => 'image_dir'
                    ),
                    'path' => '{ROOT}tmp{DS}origFiles{DS}{field}{DS}',
                    'mimetypes' => array('image/jpeg', 'image/gif', 'image/png', 'image/bmp'),
                    'extensions' => array('jpg', 'jpeg', 'JPG', 'JPEG', 'gif', 'GIF', 'png', 'PNG', 'bmp', 'BMP'),
                )
            )
        );
    }