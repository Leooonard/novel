(function(){
	var NetWorkSeg= function(){
		var ColorDict= {
			"r": "#ff0000",
			"g": "#00ff00"
		};
		var segID= 0;
		this.startX= 0;
		this.startY= 0;
		this.endX= 0;
		this.endY= 0;
		this.strokeColor= "r";
		this.conObj1= undefined;
		this.conObj2= undefined;
		this.getID= function(){
			return segID;
		};
		this.setID= function(id){
			segID= id;
		};
		this.caculatePos= function(x1, y1, x2, y2){
			x1= parseInt(x1); y1= parseInt(y1); x2= parseInt(x2); y2= parseInt(y2);
			var xGap= x2> x1? x2- x1: x1- x2;
			var yGap= y2> y1? y2- y1: y1- y2;
			var yxRatio= yGap/ xGap;
			if(x1< x2){
				if(y1< y2){
					if((yxRatio)>= 1){
						this.startX= x1+ IMG_WIDTH/ 2;
						this.endX= x2+ IMG_WIDTH/ 2;
						this.startY= y1+ IMG_HEIGHT;
						this.endY= y2;
					}else{
						this.startX= x1+ IMG_WIDTH;
						this.endX= x2;
						this.startY= y1+ IMG_HEIGHT/ 2;
						this.endY= y2+ IMG_HEIGHT/ 2;
					}
				}else if(y1== y2){
					this.startX= x1+ IMG_WIDTH;
					this.endX= x2;
					this.startY= y1+ IMG_HEIGHT/ 2;
					this.endY= y2+ IMG_HEIGHT/ 2;
				}else{
					if(yxRatio>= 1){
						this.startX= x1+ IMG_WIDTH/ 2;
						this.endX= x2+ IMG_WIDTH/ 2;
						this.startY= y1;
						this.endY= y2+ IMG_HEIGHT;
					}else{
						this.startX= x1+ IMG_WIDTH;
						this.startY= y1+ IMG_HEIGHT/ 2;
						this.endX= x2;
						this.endY= y2+ IMG_HEIGHT/ 2;
					}
				}
			}else if(x1== x2){
				this.startX= x1+ IMG_WIDTH/ 2;
				this.endX= x2+ IMG_WIDTH/ 2;
				if(y1< y2){
					this.startY= y1+ IMG_HEIGHT;
					this.endY= y2;
				}else if(y1== y2){
					this.startX= x1+ IMG_WIDTH/ 2;
					this.endX= x2+ IMG_WIDTH/ 2;
					this.startY= y1;
					this.endY= y2;
				}else{
					this.startY= y2+ IMG_HEIGHT;
					this.endY= y1;
				}
			}else{
				if(y1< y2){
					if(yxRatio>= 1){
						this.startX= x2+ IMG_WIDTH/ 2;
						this.endX= x1+ IMG_WIDTH/ 2;
						this.startY= y2;
						this.endY= y1+ IMG_HEIGHT;
					}else{
						this.startX= x2+ IMG_WIDTH;
						this.endX= x1;
						this.startY= y2+ IMG_HEIGHT/ 2;
						this.endY= y1+ IMG_HEIGHT/ 2;
					}
				}else if(y1== y2){
					this.startX= x2+ IMG_WIDTH;
					this.endX= x1;
					this.startY= y2+ IMG_HEIGHT/ 2;
					this.endY= y1+ IMG_HEIGHT/ 2;
				}else{
					if(yxRatio>= 1){
						this.startX= x2+ IMG_WIDTH/ 2;
						this.endX= x1+ IMG_WIDTH/ 2;
						this.startY= y2+ IMG_HEIGHT;
						this.endY= y1;
					}else{
						this.startX= x2+ IMG_WIDTH;
						this.endX= x1;
						this.startY= y2+ IMG_HEIGHT/ 2;
						this.endY= y1+ IMG_HEIGHT/ 2;
					}
				}
			}	
		};
		this.draw= function(mainCt){
			this.caculatePos(this.conObj1.currentRectX, this.conObj1.currentRectY, this.conObj2.currentRectX, this.conObj2.currentRectY);
			mainCt.beginPath();
			mainCt.strokeStyle= ColorDict[this.strokeColor];
			mainCt.moveTo(this.startX, this.startY);
			mainCt.lineTo(this.endX, this.endY);
			mainCt.stroke();
			mainCt.fillStyle= "black";
			mainCt.fillText(segID.toString(), (this.startX+ this.endX)/ 2+ 5, (this.startY+ this.endY)/ 2);
		};
		this.makeJSON= function(){
			var JSONObj= undefined;
			with(this){
				JSONObj= {
					"segID": segID,
					"ConObj1": conObj1.makeJSON(segID),
					"ConObj2": conObj2.makeJSON(segID)
				};
			}
			return JSONObj;
		};
		this.makeJSONForSave= function(){
			var JSONObj= undefined;
			with(this){
				JSONObj= {
					"segID": segID,
					"StartX": startX.toString(),
					"StartY": startY.toString(),
					"EndX": endX.toString(),
					"EndY": endY.toString(),
					"StrokeColor": strokeColor,
					"ConObj1": conObj1.getID(),
					"ConObj2": conObj2.getID()
				};
			}
			return JSONObj;
		};
	}  

	function Device(){
		this.img= undefined;
		this.currentRectX= 100;
		this.currentRectY= 100;                         
		this.startDragRectX=0;                        
		this.startDragRectY=0;                        
		this.startDragMouseX=0;                        
		this.startDragMouseY=0; 
		this.chosen= false;  
		this.Con_Mode= false;
		this.Div_Open= false;  
		this.info= undefined;
		this.getType= undefined;
		this.getID= undefined;		
		this.moveDiv= function(left, top){		
			this.devDiv.css("left", this.currentRectX+ left+ "px");
			this.devDiv.css("top", this.currentRectY+ IMG_HEIGHT+ top+ "px");
		};
		this.closeDiv= function(){
			this.devDiv.css("display", "none");
			this.devDiv.empty();
		};
		this.checkRect= function(mouseX, mouseY){
			if((mouseX> this.currentRectX)&& (mouseX< this.currentRectX+ IMG_WIDTH)&&
				(mouseY> this.currentRectY)&& (mouseY< this.currentRectY+ IMG_HEIGHT)){
				return this;					
			}else{
				return null;
			}
		};
		this.move= function(x, y, left, top){
			var oldX= parseInt(this.currentRectX);
			var oldY= parseInt(this.currentRectY);
			this.currentRectX= oldX+ x;
			this.currentRectY= oldY+ y;
			if(this.currentRectX+ IMG_WIDTH> MAIN_CANVAS_WIDTH){
				this.currentRectX= MAIN_CANVAS_WIDTH- IMG_WIDTH;
			}else if(this.currentRectX<= 0){
				this.currentRectX= 0;
			}
			if(this.currentRectY+ IMG_HEIGHT> MAIN_CANVAS_HEIGHT){
				this.currentRectY= MAIN_CANVAS_HEIGHT- IMG_HEIGHT;
			}else if(this.currentRectY<= 0){
				this.currentRectY= 0;
			}
			this.moveDiv(left, top);
		};
		this.draw= function(mainCt){
			mainCt.drawImage(this.img, this.currentRectX, this.currentRectY, IMG_WIDTH, IMG_HEIGHT);
		};
		this.click= function(){
			this.showDialog();
		};
		this.rightClick= function(left, top){
			if(this.Div_Open){
				this.closeDiv();
				this.Div_Open= false;
			}else{
				this.showDiv(left, top);
				this.Div_Open= true;
			}
		};
	}

	var Host= function(){
		var HostID= 0;
		this.img= new Image();
		this.img.src= "/static_media/picture/diannao.jpg";
		this.devDiv= $("<div></div>");
		this.devDiv.addClass("deviceDiv");
		this.devDiv.attr("id", "h"+ HostID);
		$(document.body).append(this.devDiv);
		this.info= new hostInfo(this);
		this.getType= function(){
			return "host";
		};
		this.getID= function(){
			return HostID;
		};
		this.setID= function(id){
			HostID= id;
			this.info.setID(HostID);
		};
		this.setSegID= function(id){
			return this.info.pushSegID(id);
		};
		this.showDiv= function(left, top){
			this.devDiv.css("left", parseInt(this.currentRectX)+ left+ "px");
			this.devDiv.css("top", parseInt(this.currentRectY)+ IMG_HEIGHT+ top+ "px");		
			var $hostdev= $($(".hostDevDiv").get(0)).clone();	
			$hostdev.find(".devID span").text(HostID.toString());
			var devtype= undefined;
			if(this.info.hostDev){
				devtype= "源设备";
			}else if(this.info.targetDev){
				devtype= "目标设备";
			}else{
				devtype= "普通设备";
			}
			$hostdev.find(".devType span").text(devtype);
			var IP= "Null";
			var Mask= "Null";
			try{
				IP= this.info.IP_Array[0];
			}catch(e){
				IP="Null";
			}
			try{
				Mask= this.info.MaskIP_Array[0];
			}catch(e){
				Mask= "Null";
			}
			$hostdev.find(".devIP span").text(IP);
			$hostdev.find(".devMask span").text(Mask);
			$hostdev.css("display", "block");
			this.devDiv.append($hostdev)
			this.devDiv.css("display", "block");
		};
		this.drop= function(){
			this.chosen= false;
		};
		this.showDialog= function(){	
			var that= this;	
			OpenHostConfDialog(this.info, [
				function(){
					setHostTarget(that.info);
				},
				redrawRect
			]);
		};
		this.makeJSON= function(segID){
			var JSONObj= {
				"HostID": HostID,
				"Type": "Host",
				"HostInfo": this.info.makeJSON(segID)
			};
			return JSONObj;
		};
		this.makeJSONForSave= function(){
			var JSONObj= {
				"HostID": HostID,
				"Type": "Host",
				"CurrentRectX": this.currentRectX.toString(),
				"CurrentRectY": this.currentRectY.toString(),
				"HostInfo": this.info.makeJSONForSave()
			};
			return JSONObj;
		};
	}

	var Route= function(){
		var RouteID= 0;
		this.img= new Image();
		this.img.src= "/static_media/picture/route.jpg";
		this.info= new routeInfo(this);
		this.devDiv= $("<div></div>");
		this.devDiv.addClass("deviceDiv");
		this.devDiv.attr("id", "r"+ RouteID);
		$(document.body).append(this.devDiv);
		this.getType= function(){
			return "route";
		};
		this.getID= function(){
			return RouteID;
		};
		this.setID= function(id){
			RouteID= id;
			this.info.setID(RouteID);
		};
		this.setSegID= function(id){
			return this.info.pushSegID(id);
		};
		this.popSegID= function(){
			this.info.popSegID();
		}
		this.showDiv= function(left, top){
			this.devDiv.css("left", parseInt(this.currentRectX)+ left+ "px");
			this.devDiv.css("top", parseInt(this.currentRectY)+ IMG_HEIGHT+ top+ "px");
			var $routeDiv= $($(".routeDevDiv").get(0)).clone();
			$routeDiv.find(".devID span").text(RouteID.toString());
			var $section= $routeDiv.find(".devSection");
			$section.remove();
			for(var i= 0; i< this.info.segID_Array.length; i++){
				var $s= $section.clone();
				$s.find(".segNumber span").text(this.info.segID_Array[i]);
				if(i< this.info.IP_Array.length){
					$s.find(".devIP span").text(this.info.IP_Array[i]);
					$s.find(".devMask span").text(this.info.MaskIP_Array[i]);
				}
				$routeDiv.append($s);
			}
			var $rtr= $routeDiv.find(".devRtr").clone();
			$routeDiv.find(".devRtr").remove();
			for(var i= 0; i< this.info.routeTable.length; i++){
				var $r= $rtr.clone();
				$r.find(".devIP span").text(this.info.routeTable[i].IP);
				$r.find(".devMask span").text(this.info.routeTable[i].MaskIP);
				$r.find(".devNexthop span").text(this.info.routeTable[i].nextSkip);
				$routeDiv.append($r);
			}
			$routeDiv.css("display", "block");
			this.devDiv.append($routeDiv);
			this.devDiv.css("display", "block");
		};
		this.drop= function(){
			this.chosen= false;
		};
		this.showDialog= function(){
			var that= this;	
			OpenRouteConfDialog(this.info, [
				redrawRect
			]);
		};
		this.makeJSON= function(segID){
			var JSONObj= {
				"RouteID": ("r"+ (parseInt(RouteID.substring(1))+ 2)),
				"Type": "Route",
				"RouteInfo": this.info.makeJSON(segID)
			};
			return JSONObj;
		};
		this.makeJSONForSave= function(){
			var JSONObj= {
				"RouteID": ("r"+ (parseInt(RouteID.substring(1))+ 2)),
				"Type": "Route",
				"CurrentRectX": this.currentRectX.toString(),
				"CurrentRectY": this.currentRectY.toString(),
				"RouteInfo": this.info.makeJSONForSave()
			};
			return JSONObj;
		}
	}  
	Host.prototype= new Device();
	Route.prototype= new Device();

	window.NetWorkSeg= NetWorkSeg;
	window.Host= Host;
	window.Route= Route;
	window.Device= Device;
	
})();