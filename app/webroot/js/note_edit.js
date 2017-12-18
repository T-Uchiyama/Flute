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
    var generated_meta_info = [];
    
    var id_prefix = 'tile';
    var preload_id_prefix = 'preload_tile';
    var file_prefix = '/read/test.bmp';
    
    var htm = '';
    // 指定範囲
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
        htm += gen_img_tag(id_prefix, (y + t_ost_y), (x + t_ost_x), preload_id_prefix, w, h, tile_pos_y, tile_pos_x);
        generated_meta_info.push({prefix:id_prefix, y:(y + t_ost_y), x: (x + t_ost_x)});
      }
    }
    $('#draw_window').html(htm);

    htm = '';
    // Preload範囲
    for (p = 0 ; p < pre_load_size ; p++){
      for (x = 0 ; x < tile_x ; x++){
        var pre_y = t_ost_y -1 - p;
        var pre_x = x + t_ost_x;
        htm += gen_img_tag(preload_id_prefix, pre_y, pre_x);
        generated_meta_info.push({prefix:preload_id_prefix, y:pre_y, x: pre_x});
        pre_y = tile_y + t_ost_y + p;
        pre_x = x + t_ost_x;
        htm += gen_img_tag(preload_id_prefix, pre_y, pre_x);
        generated_meta_info.push({prefix:preload_id_prefix, y:pre_y, x: pre_x});
      }
      for (y = - pre_load_size ; y < tile_y + pre_load_size ; y++){
        var pre_y = y + t_ost_y;
        var pre_x = t_ost_x -1 - p;
        htm += gen_img_tag(preload_id_prefix, pre_y, pre_x);
        generated_meta_info.push({prefix:preload_id_prefix, y:pre_y, x: pre_x});
        pre_y = y + t_ost_y;
        pre_x = tile_x + t_ost_x + p;
        htm += gen_img_tag(preload_id_prefix, pre_y, pre_x);
        generated_meta_info.push({prefix:preload_id_prefix, y:pre_y, x: pre_x});
      }
    }
    $('#pre_load_area').html(htm);
    
    generated_meta_info.forEach(function (o){
      combine_fpath_id(o.prefix, file_prefix, o.y , o.x);
    });
  };
  /**
   * Image用のhtmlタグを生成
   * @param  id_prefix    id定義
   * @param  tile_y       指定されているy座標
   * @param  tile_x       指定されているx座標
   * @param  {String} [preload_id_prefix=''] Preload範囲用id定義 default = ''
   * @param  {Number} [width=0]              画像のサイズ:横 default = 0
   * @param  {Number} [height=0]             画像のサイズ:縦 default = 0
   * @param  {Number} [pos_y=0]              y座標の表示開始位置 default = 0
   * @param  {Number} [pos_x=0]              x座標の表示開始位置 default = 0
   */
  function gen_img_tag(id_prefix, tile_y, tile_x, preload_id_prefix = '', width = 0, height = 0, pos_y = 0, pos_x = 0){
    // x座標またはy座標が0以下で画像サイズが縦横0の場合は範囲外のため関数終了。
    if ((tile_x < 0 || tile_y < 0) && width == 0 && height == 0 ){
       return '';
    }
    var htm = '';
    var id = id_prefix + '_' + tile_y + '_' + tile_x;
    var src = '';
    // 既に同様のIDが存在しており、srcが記載されている場合には記載されているsrcを取得。
    if ($('#' + id).length && $('#' + id).attr('src') != undefined && $('#' + id).attr('src').length){
      src = $('#' + id).attr('src');
    }
    // 通常範囲
    if (width != 0 && height != 0){
      var preload_id = preload_id_prefix + '_' + tile_y + '_' + tile_x;
      var img_style = 'pointer-events: none;';
      // pos_xとpos_yの位置調整
      if (pos_x < 0){
        pos_x = 0;
        img_style += 'right:0;'
      }
      if (pos_y < 0){
        pos_y = 0;
        img_style += 'bottom:0;'
      }
      // img_styleにposition:absoluteを追加
      if (img_style.length){
        img_style = 'style="position:absolute;' + img_style +'"';
      }
      // srcが記載されておらず、Preload用のIDが存在し、既にsrc生成が終了している場合はPreloadIDを使用しsrcを取得
      if (src.length == 0 && $('#' + preload_id).length && $('#' + preload_id).attr('src') != undefined && $('#' + preload_id).attr('src').length){
        src = $('#' + preload_id).attr('src');
      }
      // 以前のsrcがある場合にはそのまま流用
      if (src.length){
        src = ' src="' + src + '"';
      }
      // 通常範囲のタグ生成
      htm += '<div style="width:' + width + 'px;height:' + height + 'px;position:absolute;left:'+ pos_x +'px;top:'+ pos_y +'px;overflow:hidden;">';
      htm += '<img id="' + id + '" width="'+ tile_w + '"height="'+ tile_h +'"' + img_style + src +' >\n';
      htm += '</div>';
    } else {
        //Preload範囲の場合にはsrcをダイレクトで作成。
      if (src.length){
        src = ' src="' + src + '"';
      }
      // Preload範囲のタグ生成
      htm += '<img id="' + id + '"' + src +' >\n';
    }
    return htm;
  }
  /**
   * 配列の要素よりファイルパスを結合・生成し、画像読み込みを実施。
   * @param  id_prefix    id定義
   * @param  fpath_prefix ファイルパス定義
   * @param  tile_y       指定されているy座標
   * @param  tile_x       指定されているx座標
   */
  function combine_fpath_id(id_prefix, fpath_prefix, tile_y, tile_x){
    // x座標またはy座標が0以下の場合には描画不必要な範囲のため関数を終了させる。
    if (tile_x < 0 || tile_y < 0){
       return;
    }
    // idとurlの生成。
    var id = id_prefix + '_' + tile_y + '_' + tile_x;
    var url = fpath_prefix + '_y' + tile_y + '_x' + tile_x + '.png';
    // 既に同様のIDが存在しており、読み込みが成功している場合には読み込み不必要であるため関数終了させる。
    if (($('#' + id).attr('src')) != undefined && ($('#' + id).attr('src')).length){
      return;
    }
    // 画像読み込みを実施。
    load_img(url, id);
  }
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