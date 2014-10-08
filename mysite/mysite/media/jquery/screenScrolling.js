(function(){
     /*
          该JS用来做滚屏动画效果.
          元素的类中包含screen_scrolling的, 即为一个屏幕.

          1. 收集所有屏幕. 放置数组中.
          2. 监听滚动事件. 触发时, 则发生动画效果.
          3. 动画发生时, 不再监听事件.
          4. 动画形式默认是上下. 可以接受用户定制.

          调用过程:
          1. 生成screenScrolling对象.
          2. collect(), 收集所有屏幕.
          3. bind(), 绑定事件, 开始监听.
          4. addAnimation(), 添加用户定制动画.
     */

     var screenScrolling= function(){
          //屏幕列表. 存放的是jquery对象.
          var screenArray= new Array();
          //现在的屏幕.
          var currentScreenCount= 0;
          //是否正在动画阶段.
          var rolling= false;
          var windowHeight= document.body.offsetHeight;
          //用户定义的动画. 可接受一个currentScreen参数, 表明是第几个屏幕, 是函数类型, 返回动画对象.
          var userAnimation= undefined;

          this.collect= function(){
               var $screen= $(".screen_scrolling");
               for(var i= 0; i< $screen.length; i++){
                    screenArray.push($($screen.get(i)));
               }
          };

          //默认动画时间.
          this.animationTime= 2000;
          this.addAnimation= function(animation){
               userAnimation= animation;
          };

          var animation= function(currentScreen){
               if(userAnimation){
                    return userAnimation(currentScreen);
               }
               var height= (currentScreen* windowHeight).toString()+ "px";
               var animation= {
                    scrollTop: height
               };
               return animation;
          };

          //动画结束后的回调.
          this.animationCallback= undefined;
          var that= this;
          this.bind= function(){
               var func= function(e){
                    //正在动画过程则不接受事件.
                    if(rolling){
                         return false;
                    }
                    var event= e|| window.event;
                    //解决不同浏览器间的不同.
                    var wheelDelta= event.wheelDelta|| -(event.detail);

                    //判断是否接受鼠标滚动事件.
                    //第一个屏幕不接受向上滚动. 最后一个屏幕不接受向下滚动.
                    //接受则设置rolling为true.
                    if(wheelDelta< 0){
                         //滚轮向下滚.
                         if(currentScreenCount< screenArray.length- 1){
                              currentScreenCount= currentScreenCount+ 1;
                              rolling= true;
                         }
                    }else{
                         //滚轮向上滚.
                         if(currentScreenCount> 0){
                              currentScreenCount= currentScreenCount- 1;
                              rolling= true;
                         }
                    }
                    //开始滚动动画. 通过调用函数获得滚动动画对象.
                    if(rolling){
                         $(document.body).animate(animation(currentScreenCount), that.animationTime, function(){
                              if(this.animationCallback){
                                   this.animationCallback();
                              }
                              rolling= false;
                         });
                    }
               };   
               
               if (window.addEventListener) {
                    if (document.mozHidden !== undefined) {
                         type = "DOMMouseScroll";
                    }else{
                         type= "mousewheel";
                    }
                    document.addEventListener(type, function(e){
                         func(e);
                    });
               }else if (window.attachEvent){
                    document.attachEvent("onmousewheel", function(e){
                         func(e);
                    });
               }                           
          };
     };

     window.screenScrolling= screenScrolling;
})();