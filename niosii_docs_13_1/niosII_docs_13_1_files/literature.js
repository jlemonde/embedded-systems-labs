function newImage(arg) {
	if (document.images) {
		rslt = new Image();
		rslt.src = arg;
		return rslt;
	}
}

function changeImages() {
	if (document.images && (preloadFlag == true)) {
		for (var i=0; i<changeImages.arguments.length; i+=2) {
			document[changeImages.arguments[i]].src = changeImages.arguments[i+1];
		}
	}
}

var preloadFlag = false;
function preloadImages(image_path) {
	if (document.images) {
		buy_button2_01_over = newImage(image_path);
		preloadFlag = true;
	}
}


/*************************************************************************************************************
 * The following functions are used in the expand/collapse function in literature HandBood listing(Cyc3, stx3)
 * we can use either text or image as expand/collapse icon. 
 * Erin: 2007-1-31
 		 The collapsable HB listing must folllow the following html format:
 		 <div class='collapsable'>
			  <h4>"Application Notes" or "Volume 1 - Stratix III Device Handbook"</h4>
				  <span>
						text here
				  </span>
		 </div>
***************************************************************************************************************/

var COLLAPSABLE_PARENT_NAME = "literature-hb-collapsable";

//collapsableinit() function - Initializes hide/show whitch Content function (collapse contents by default)
collapsableinit=function(){
	if(document.getElementById && document.createTextNode) { 
	    // returns an array of elements with a name attribute whose value matches specified classname parameter
        var entries = document.getElementsByTagName(COLLAPSABLE_PARENT_TYPE);  
        for(i=0;i<entries.length;i++) 
			if (entries[i].className==COLLAPSABLE_PARENT_NAME)  {
                assignCollapse(entries[i]);	
			}
    } 
}


//assign collapse/expand text to each block
function assignCollapse(div) { 
	//creates an instance of the element object with tag name "a"
    var collapsableIcon = document.createElement('a'); 
	collapsableIcon.className = 'literature-hb-twisty';
	//DOM Style Object:represents an individual style statement
	//cursor: sets the type of cursor to be displayed.
	//        example: value=pointer -- the cursor indicates a link
	//                   value=text - the cursor indicates text
    collapsableIcon.style.cursor='pointer'; 
	//create a e/c attribute, ready to be inserted somewhere in document
    collapsableIcon.setAttribute('expand', COLLAPSABLE_EXPAND); 
    collapsableIcon.setAttribute('shrink', COLLAPSABLE_SHRINK);
	
	//default to collapse all
    //collapsableIcon.setAttribute('state', -1); 
	
	//-- For 20080519 launch, we need to collapse all sections for lit-qts.jsp handbook listing, but for other handbook listing still 
	//-- keep Section expanded
	//-- step 1: find the file name for lit-qts.jsp: 
	//   		-- gets the URL of the current page, then extracts the file name and path from the URL
	//-- step 2: if is Q2 handbook listing,  set the flat and then collapse all section fields.
	//-- 20091204 lit-external-memory-interface.jsp & lit-nio2.jsp to have same default settings as lit-qts.jsp
	var URL = unescape(location.href)	// get current URL in plain ASCII
	var myfilename = URL.substring(URL.lastIndexOf("/") + 1,URL.length)

	//For literature handbook use only: volume section default to expanded and related doc default to collapse
	//if (div.getElementsByTagName(COLLAPSABLE_START_TYPE)[0].className=='literature-hb-topic') {
		
	// 2008-0519: for Q2 literature handboook: volume section default to expanded but the section potion default to collapsed 
	if (div.getElementsByTagName(COLLAPSABLE_START_TYPE)[0].className=='literature-hb-topic' || (div.getElementsByTagName(COLLAPSABLE_START_TYPE)[0].className=='literature-hb-section' && (myfilename=='lit-qts.jsp' || myfilename=='lit-external-memory-interface.jsp' || myfilename=='lit-nio2.jsp'))) {
		collapsableIcon.setAttribute('state', -1); 
	} else {
    	collapsableIcon.setAttribute('state', 1); 
	}
	
	//syntax: fatherObj.insertBefore(childObj, brotherObj)
	div.insertBefore(collapsableIcon, div.getElementsByTagName(COLLAPSABLE_START_TYPE)[0]);
	collapsableIcon.onclick=function() { twisty(collapsableIcon, 0); }               
    collapsableIcon.onclick(); 
	

} 



/* -- since The window.onload event is used by programmers to kick-start their web applications.
   -- The problem is that the onload event fires after all page content has loaded - for Mozilla browser, it may
   -- have lag before the page active. To fix it: must let DOM has fully loaded without waiting.  Mozilla provides an enent 
   -- tailor-made for this (Opera 9 also support this) : DOMContentLoaded. and can fix above issue for Mozilla browser. 
   -- For other browser, use standard window.onload event. 
   -- The addEventListener method takes the event as the first parameter, the event handler (function) as the second, and whether 
   -- the event is captured or allowed to "bubble up" to other events with the third parameter.
*/    
   if (document.addEventListener) {    
		document.addEventListener("DOMContentLoaded", collapsableinit, false);
   } /*else {
	 	window.onload=collapsableinit;
   }*/
   
   /* for Internet Explorer */
  document.write("<script id=__ie_onload defer src=javascript:void(0)><\/script>");
  var script = document.getElementById("__ie_onload");
  script.onreadystatechange = function() {
    if (this.readyState == "complete") {
      collapsableinit(); // call the onload handler
    }
  };
/* end */
