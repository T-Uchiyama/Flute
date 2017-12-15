$(function(){
  var w_width = $('#draw_window').width();
  var w_height = $('#draw_window').height();
  var cam_x = 0;
  var cam_y = 0;
  var mag = 1.0; /* 表示倍率 */
  var tile_w = 256;
  var tile_h = 256;
  
  var mx = 0;
  var my = 0;
  var drag = false;
  //console.log('width:'+w_width);
  //console.log('height:'+w_height);
  
  function load_img(url, id){
    //imgPreloaderオブジェクトを作る、名前は任意
    var imgPreloader=new Image();
    //onloadイベントハンドラ追加
    imgPreloader.onload=function() {
      //ロード完了で画像を表示
      $('#'+id).attr({'src':url});
    }
    //重要、最後に書く
    imgPreloader.src=url;
  }
  
  function draw() {
    var ost_x = ((cam_x % tile_w) + tile_w) % tile_w;
    var ost_y = ((cam_y % tile_h) + tile_h) % tile_h;
    var tile_x = Math.floor(((w_width + ost_x) + tile_w - 1) / tile_w);
    var tile_y = Math.floor(((w_height + ost_y) + tile_h - 1) / tile_h);
    var x;
    var y;
    var t_ost_x = Math.floor(cam_x / tile_w);
    var t_ost_y = Math.floor(cam_y / tile_h);
    var pre_load_size = 2;
    
    var htm = '';
    for (y = 0 ; y < tile_y ; y++){
      for (x = 0 ; x < tile_x ; x++){
        var w = tile_w;
        var h = tile_h;
        
        if (x * tile_w - ost_x < 0){
          w = tile_w - ost_x;
        }
        if (y * tile_h - ost_y < 0){
          h = tile_h - ost_y;
        }
        if (x * tile_w + w - ost_x > w_width){
          w = w_width - x * tile_w + ost_x;
        }
        if (y * tile_h + h - ost_y > w_height){
          h = w_height - y * tile_h + ost_y;
        }
        if (w < 0 || h < 0){
          continue;
        }
        var tile_pos_x = (tile_w*x-ost_x);
        var tile_pos_y = (tile_h*y-ost_y);
        var img_style = 'pointer-events: none;';
        if (tile_pos_x < 0){
          tile_pos_x = 0;
          img_style += 'right:0;'
        }
        if (tile_pos_y < 0){
          tile_pos_y = 0;
          img_style += 'bottom:0;'
        }
        htm += '<div style="width:' + w + 'px;height:' + h + 'px;position:absolute;left:'+ tile_pos_x +'px;top:'+ tile_pos_y +'px;overflow:hidden;">';
        if (img_style.length){
          img_style = 'style="position:absolute;' + img_style +'"';
        } 
        var id = 'tile_' + (y + t_ost_y) + '_' + (x + t_ost_x);
        var src = '';
        if ($('#' + id).length && $('#' + id).attr('src') != undefined && $('#' + id).attr('src').length){
          src = $('#' + id).attr('src');
        }
        var preload_id = 'preload_tile_' + (y + t_ost_y) + '_' + (x + t_ost_x);
        if ($('#' + preload_id).length && $('#' + preload_id).attr('src') != undefined && $('#' + preload_id).attr('src').length){
          src = $('#' + preload_id).attr('src');
        }
        if (src.length){
          src = ' src="' + src + '"';
        }
        htm += '<img id="' + id + '" width="'+ tile_w + '"height="'+ tile_h +'"' + img_style + src +' >\n';
      
        htm += '</div>';
        //console.log(x);
        //console.log(y);
      }
    }
    $('#draw_window').html(htm);
    
    for (y = 0 ; y < tile_y ; y++){
      for (x = 0 ; x < tile_x ; x++){
        var w = tile_w;
        var h = tile_h;
        if (x * tile_w + w - ost_x > w_width){
          w = w_width - x * tile_w + ost_x;
        }
        if (y * tile_h + h - ost_y > w_height){
          h = w_height - y * tile_h + ost_y;
        }
        if (w < 0 || h < 0){
          continue;
        }
        if ((y + t_ost_y) < 0 || (x + t_ost_x) < 0){
            continue;
        }
        //console.log('y:'+ (y + t_ost_y) + ' x:' + (x + t_ost_x));
        var id = 'tile_' + (y + t_ost_y) + '_' + (x + t_ost_x);
        var url = '/read/test.bmp_y' + (y + t_ost_y) + '_x' + (x + t_ost_x) + '.png';
        if (($('#' + id).attr('src')) != undefined && ($('#' + id).attr('src')).length){
          continue;
        }
        load_img(url, id);
      }
    }
    
    // preload 
    htm = '';
    for (var cnt = 0; cnt < pre_load_size; cnt++){
      for (x = 0 ; x < tile_x ; x++){
        var pre_y = t_ost_y -1 - cnt;
        var pre_x = x + t_ost_x;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var src = '';
        if ($('#' + id).length && $('#' + id).attr('src') != undefined && $('#' + id).attr('src').length){
          src = $('#' + id).attr('src');
        }
        if (src.length){
          src = ' src="' + src + '"';
        }
        htm += '<img id="' + id + '"' + src +' >\n';
      }
      
      for (x = 0 ; x < tile_x ; x++){
        var pre_y = tile_y + t_ost_y + cnt;
        var pre_x = x + t_ost_x;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var src = '';
        if ($('#' + id).length && $('#' + id).attr('src') != undefined && $('#' + id).attr('src').length){
          src = $('#' + id).attr('src');
        }
        if (src.length){
          src = ' src="' + src + '"';
        }
        htm += '<img id="' + id + '"' + src +' >\n';
      }
      
      for (y = - pre_load_size ; y < tile_y + pre_load_size ; y++){
        var pre_y = y + t_ost_y;
        var pre_x = t_ost_x -1 - cnt;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var src = '';
        if ($('#' + id).length && $('#' + id).attr('src') != undefined && $('#' + id).attr('src').length){
          src = $('#' + id).attr('src');
        }
        if (src.length){
          src = ' src="' + src + '"';
        }
        htm += '<img id="' + id + '"' + src +' >\n';
      }
      
      for (y = - pre_load_size ; y < tile_y + pre_load_size ; y++){
        var pre_y = y + t_ost_y;
        var pre_x = tile_x + t_ost_x + cnt;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var src = '';
        if ($('#' + id).length && $('#' + id).attr('src') != undefined && $('#' + id).attr('src').length){
          src = $('#' + id).attr('src');
        }
        if (src.length){
          src = ' src="' + src + '"';
        }
        htm += '<img id="' + id + '"' + src +' >\n';
      }
    }
    
    $('#pre_load_area').html(htm);
    
    for (cnt = 0 ; cnt < pre_load_size ; cnt++){
      for (x = 0 ; x < tile_x ; x++){
        var pre_y = t_ost_y -1 - cnt;
        var pre_x = x + t_ost_x;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var url = '/read/test.bmp_y' + pre_y + '_x' + pre_x + '.png';
        if (($('#' + id).attr('src')) != undefined && ($('#' + id).attr('src')).length){
          continue;
        }
        load_img(url, id);
      }
      
      for (x = 0 ; x < tile_x ; x++){
        var pre_y = tile_y + t_ost_y + cnt;
        var pre_x = x + t_ost_x;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var url = '/read/test.bmp_y' + pre_y + '_x' + pre_x + '.png';
        if (($('#' + id).attr('src')) != undefined && ($('#' + id).attr('src')).length){
          continue;
        }
        load_img(url, id);
      }
    
      for (y = - pre_load_size ; y < tile_y + pre_load_size ; y++){
        var pre_y = y + t_ost_y;
        var pre_x = t_ost_x -1 - cnt;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var url = '/read/test.bmp_y' + pre_y + '_x' + pre_x + '.png';
        if (($('#' + id).attr('src')) != undefined && ($('#' + id).attr('src')).length){
          continue;
        }
        load_img(url, id);
      }

      for (y = - pre_load_size ; y < tile_y + pre_load_size ; y++){
        var pre_y = y + t_ost_y;
        var pre_x = tile_x + t_ost_x + cnt;
        if (pre_x < 0 || pre_y < 0){
           continue;
        }
        var id = 'preload_tile_' + pre_y + '_' + pre_x;
        var url = '/read/test.bmp_y' + pre_y + '_x' + pre_x + '.png';
        if (($('#' + id).attr('src')) != undefined && ($('#' + id).attr('src')).length){
          continue;
        }
        load_img(url, id);
      }
    }
  };
  $('#draw_window').mousedown(function (e){
    mx = e.pageX - this.offsetLeft;
    my = e.pageY - this.offsetTop;
    drag = true;
  });
  
  $('#draw_window').mousemove(function (e){
    var p_mx = mx;
    var p_my = my;
    mx = e.pageX - this.offsetLeft;
    my = e.pageY - this.offsetTop;
    if (drag){
      cam_x += p_mx - mx;
      cam_y += p_my - my;
      draw();
    }
  });
  $('#draw_window').mouseup(function (e){
    mx = e.pageX - this.offsetLeft;
    my = e.pageY - this.offsetTop;
    drag = false;
  });
  $('#draw_window').mouseleave(function (e){
    mx = 0;
    my = 0;
    drag = false;
  });
  var timer = false;
  $(window).resize(function() {
      if (timer !== false) {
          clearTimeout(timer);
      }
      timer = setTimeout(function() {
         w_width = $('#draw_window').width();
         w_height = $('#draw_window').height();
         draw();
         //console.log('width:'+w_width);
         //console.log('height:'+w_height);
          // 何らかの処理
      }, 200); // リサイズ中かどうか判定する時間、ms
  });
  
  
  

  
  draw();
  
});