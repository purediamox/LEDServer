"use strict";

function set_Color(color)
{
    console.log("api/setcolor(" + color + ")");
    console.log(color);
    $.get("api/setcolor", 
    { "color" : color }).done(
        function(data) { }          // ignore the result.
    );
}


function init()
{
    console.log("init");
    //var parent = document.querySelector('#parent');
    //var picker = new Picker(parent);

    /*
        You can do what you want with the chosen color using two callbacks: onChange and onDone.
    */
    //picker.onChange = function(color) {
    //    parent.style.background = color.rgbaString;
    //    console.log(color);
    //    set_Color(color.rgbaString);
    //};
}