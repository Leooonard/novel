(function(){
	var InitialDeviceInfo= function(){
		var rImg= new Image();
		rImg.src= "/static_media/picture/route.jpg";
		var hImg= new Image();
		hImg.src= "/static_media/picture/diannao.jpg";
		var inRect= function(){
			
		};
		this.initialRouteDevice= {
			"img": rImg,
			"x": 10,
			"y": 10,
			"width": 50,
			"height": 50
		};
		this.initialHostDevice= {
			"img": hImg,
			"x": 10,
			"y": 110,
			"width": 50,
			"height": 50
		};
	};



	function Info(){
		this.hostDev= false;
		this.targetDev= false;
		this.pushSegID= undefined;
		this.makeJSONString= undefined;
		this.makeJSONStringForSave= undefined;
		this.parent= undefined;
	}

	function hostInfo(parent){
		this.segID_Array= [];
		this.IP_Array= [];
		this.MaskIP_Array= [];
		this.parent= parent;
		this.HostID= undefined;
		this.pushSegID= function(id){
			if(this.segID_Array.length> 0){
				return false;
			}else{
				this.segID_Array.push(id);
				return true;
			}
		};
		this.getID= function(){
			return this.HostID;
		};
		this.setID= function(id){
			this.HostID= id;
		};
		this.makeJSON= function(segID){
			var JSONStr= undefined;
			var JSONObj= undefined;
			JSONObj= {
				"HostDev": this.hostDev,
				"TargetDev": this.targetDev
			};
			JSONObj["IP"]= this.IP_Array.length> 0? this.IP_Array[0]: "Null";
			JSONObj["MaskIP"]= this.MaskIP_Array.length> 0? this.MaskIP_Array[0]: "Null";
			return JSONObj;
		};
		this.makeJSONForSave= function(){
			var JSONStr= undefined;
			var JSONObj= undefined;
			JSONObj= {
				"HostDev": this.hostDev,
				"TargetDev": this.targetDev,
			};
			JSONObj["SegID"]= this.segID_Array.length> 0? this.segID_Array[0]: "Null";
			JSONObj["IP"]= this.IP_Array.length> 0? this.IP_Array[0]: "Null";
			JSONObj["MaskIP"]= this.MaskIP_Array.length> 0? this.MaskIP_Array[0]: "Null";
			return JSONObj;
		};
	}

	function routeInfo(parent){
		this.segID_Array= [];
		this.IP_Array= [];
		this.MaskIP_Array= [];
		this.parent= parent;
		this.routeID= undefined;
		this.routeTable= [];
		this.pushSegID= function(id){
			this.segID_Array.push(id);
			return true;
		};
		this.popSegID= function(){
			this.segID_Array.pop();
		};
		this.getID= function(){
			return this.routeID;
		};
		this.setID= function(id){
			this.routeID= id;
		};
		this.makeRTR= function(){
			return new routeTableRow();
		};
		this.makeJSON= function(segID){
			var JSONObj= undefined;
			var IP= "";
			var MaskIP= "";
			var routeTableObj= [];
			with(this){	
				for(var i= 0; i< segID_Array.length; i++){
					if(segID_Array[i]== segID){
						if(i< IP_Array.length){
							IP= IP_Array[i];
							MaskIP= MaskIP_Array[i];
						}
						break;
					}
				}
				for(var i= 0; i< routeTable.length; i++){
					routeTableObj.push(routeTable[i].makeJSON());
				}
				JSONObj= {
					"HostDev": hostDev,
					"TargetDev": targetDev,
					"RouteTable": routeTableObj
				};
				JSONObj["IP"]= IP|| "Null";
				JSONObj["MaskIP"]= IP? MaskIP: "Null";
			}
			return JSONObj;
		};
		this.makeJSONForSave= function(){
			var routeTableObj= [];
			with(this){
				for(var i= 0; i< routeTable.length; i++){
					console.log(routeTable[i]);
					routeTableObj.push(routeTable[i].makeJSON());
				}
				var JSONObj= {
					"SegID_Array": segID_Array,
					"IP_Array": IP_Array,
					"MaskIP_Array": MaskIP_Array,
					"HostDev": hostDev,
					"TargetDev": targetDev,
					"RouteTable": routeTableObj
				};
			};
			return JSONObj;
		};
	}

	function routeTableRow(){
		this.IP= undefined;
		this.MaskIP= undefined;
		this.nextSkip= undefined;
		this.makeJSON= function(){
			var JSONObj= undefined;
			JSONObj= {
				"IP": this.IP,
				"NextSkip": this.nextSkip,
				"MaskIP": this.MaskIP
			};
			return JSONObj;
		};
	}

	
	hostInfo.prototype= new Info();
	routeInfo.prototype= new Info();

	window.hostInfo= hostInfo;
	window.routeInfo= routeInfo;
	window.routeTableRow= routeTableRow;
	window.initialDeviceInfo= new InitialDeviceInfo();
})();

