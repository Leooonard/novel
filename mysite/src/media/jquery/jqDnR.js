//对于几个mouse事件的绑定, 有可能有问题!

(function($){
	$.fn.jqDrag= function(c, handler){
		callback= c;
		return i(this, callback, handler, 'drag');
	};
	$.fn.jqResize= function(callback, handler){
		return i(this, null, handler,'resize');
	};
	$.fn.jqStopDrag= function(){
		J.stop();
	};
	$.fn.jqSetPxPy= function(x, y){
		pageX= x; pageY= y;
	};
	$.jqDnR={
		dnr: {}, e: 0,
		drag:function(event){
		 	if(M.op== 'drag'){
		 		Ele.css('left', M.X+ event.pageX- M.pX);
		 		Ele.css('top', M.Y+ event.pageY- M.pY);
			 	if(callback!= null){
			 		callback(H, 0);
			 	}
		 	}else if(M.op== 'resize'){
		 		Ele.css('width', Math.max(event.pageX- M.pX+ M.W, 0));
		 		Ele.css('height', Math.max(event.pageY- M.pY+ M.H, 0));
		 	}
		  	return false;
	  	},
		stop:function(){
			Ele.css('opacity', M.o);
			Ele.unbind('mousemove', jObject.drag).unbind('mouseup', jObject.stop);
			return false;
	  	}
	};
	var jObject=$.jqDnR, Mouse= jObject.dnr, E= jObject.e, pageX, pageY, callback,
	i= function(element, callback, handler, op){
		return element.each(function(){
			handler= (handler)? $(handler, element): element;
			H= handler;
	 		handler.bind('mousedown', {e: element, k: op},function(event){
	 			var data= event.data; 
	 			Ele= data.e;
				// attempt utilization of dimensions plugin to fix IE issues
	 			if(Ele.css('position')!= 'relative'){
	 				try{
	 					Ele.css('position', 'relative');
	 				}catch(e){}
	 			}
	 			M={
	 			    X: f('left')|| 0,
	 			    Y: f('top')|| 0, 
	 			    W: f('width')|| E[0].scrollWidth|| 0,
	 			    H: f('height')|| E[0].scrollHeight|| 0, 
	 			    pX: event.pageX, pY: event.pageY, 
	 			    op: data.k, 
	 			    o: Ele.css('opacity')
	 			};
		 		Ele.css('opacity', '0.8');
	 			if(event.pageX== null){
	 				Mouse.pX= pageX;
	 				Mouse.pY= pageY;
	 			}
	 			Ele.mousemove($.jqDnR.drag).mouseup($.jqDnR.stop);
	 			return false;
	 		});
		});
	},
	f= function(attr){
		return parseInt(Ele.css(attr))||false;
	};
})(jQuery);
