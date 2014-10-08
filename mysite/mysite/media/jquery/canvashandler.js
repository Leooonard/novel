(function(){
	/*
		CanvasMouseHandler　用于处理鼠标事件的模块.
		1. 帮助绑定事件.
		2. 处理按下, 移动, 抬起三个事件.
		3. 处理单击事件(即在原处按下抬起的事件).
		4. 单个handler只负责处理一个对象.
		5. 需要调用者提供2个回调函数, 三个接口.
	*/	

	var CanvasMouseHandler= function(){
		var bindTarget= undefined;
		var mouseDownCallback= function(){};
		var mouseMoveCallback= function(){};
		var mouseUpCallback= function(){};
		var redrawCallback= function(){};
		var mouseDownX= undefined;
		var mouseDownY= undefined;
		var chosenObject= undefined;
		var clickEvent= true; //判断是否是单击事件.

		var bias= {
			"left": 0,
			"top": 0
		};

		var stepSize= 1; //步距, 为x则是每x次触发move事件移动.
		this.init= function(dict){
			if(dict["bias"]){
				bias.left= Number(dict["bias"].left);
				bias.top= Number(dict["bias"].top);
			}
			stepSize= dict["stepSize"]|| stepSize;
			stepSize= parseInt(stepSize);
			mouseDownCallback= dict["mdcallback"]|| mouseDownCallback;
			mouseMoveCallback= dict["mmcallback"]|| mouseMoveCallback;
			mouseUpCallback= dict["mucallback"]|| mouseUpCallback;
			redrawCallback= dict["rdcallback"]|| redrawCallback;
		};

		var UnInit= function(){		
			chosenObject= undefined;
			clickEvent= true;
			mouseDownX= undefined;
			mouseDownY= undefined;
		}

		this.bind= function(target){
			bindTarget= $(target);

			//绑定三个事件.
			bindTarget.mousedown(MouseDownHandler);
			bindTarget.mousemove(MouseMoveHandler);
			bindTarget.mouseup(MouseUpHandler);
			bindTarget.bind("contextmenu", function(){
				return false;
			});
		};

		var MouseDownHandler= function(e){
			var event= e|| window.event;
			var $this= $(this);
			mouseDownX= event.pageX- bias.left;
			mouseDownY= event.pageY- bias.top;
			//传入的回调函数必须返回一个被选中的元素.
			chosenObject= mouseDownCallback(mouseDownX, mouseDownY);
		};

		var MouseMoveHandler= function(e){
			if(chosenObject){ //当选中对象不为空时, 才触发拖动事件.
				var event= e|| window.event;
				var $this= $(this);
				var mouseMoveX= event.pageX- bias.left;
				var mouseMoveY= event.pageY- bias.top;
				clickEvent= false; //进入该事件说明不可能是单击事件.

				if(mouseMoveCallback&& mouseMoveCallback(chosenObject)=== false){
					return false;
				}
				if(MouseMoveHandler.step== stepSize){
					//move为定义的接口, 对象必须实现, 传入的是移动的距离, 根据该距离进行坐标转换.
					try{
						chosenObject.move(mouseMoveX- mouseDownX, mouseMoveY- mouseDownY, bias.left, bias.top);
					}catch(e){
						return false;
					}
					mouseDownX= mouseMoveX;
					mouseDownY= mouseMoveY;
					//移动之后, 必须进行重绘.
					redrawCallback();

					MouseMoveHandler.step= 1;
				}else{
					MouseMoveHandler.step+= 1;
				}
			}
		}
		MouseMoveHandler.step= 1;

		var MouseUpHandler= function(e){
			//up事件比较特殊, 包括了拖动up和单击up.
			var event= e||window.event;
			var $this= $(this);
			var mouseUpX= event.pageX- bias.left;
			var mouseUpY= event.pageY- bias.top;
			//如果注册了鼠标抬起回调, 就调用, 如果返回false, 则不执行默认事件.
			if(mouseUpCallback&& mouseUpCallback(chosenObject)=== false){
				UnInit();
				return;
			}
			if(clickEvent){
				//单击事件
				MouseUpClickHandler(event);
			}else{
				//拖动事件
				MouseUpDragHandler(event);
			}
		}

		var MouseUpDragHandler= function(e){
			if(chosenObject){
				try{
					chosenObject.drag(); //drag是定义的接口. 可选实现.
				}catch(e){
					;
				}
			}
			UnInit();
		};

		var MouseUpClickHandler= function(e){
			if(chosenObject){
				try{
					if(e.button== 0){  //左键
						chosenObject.click(); //click也是定义的接口, 必须实现.
					}else if(e.button== 2){ //右键
						chosenObject.rightClick(bias.left, bias.top); //rightClick是可选接口.
					}
				}catch(e){
					;
				}
			}
			UnInit();
		}
	};

	
	window.CanvasMouseHandler= CanvasMouseHandler;
})();