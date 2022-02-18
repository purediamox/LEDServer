"use strict";

function set_effect(id) 
{
    console.log("set_effect");
    console.log(id);
    $.get("api/seteffect", 
    { "id" : id }).done(
        function(data) { }          // ignore the result - TODO this will obtain the settings
    );
}


function set_Color(color)
{
    console.log("api/setcolor(" + color + ")");
    console.log(color);
    $.get("api/setcolor", 
    { "color" : color }).done(
        function(data) { }          // ignore the result.
    );
}

function populate_effects()
{
    $.get("api/geteffects").done(function(data) {
        console.log(data);
        $("#sel_user").empty();
        let effects = data['effects'];
        let len = effects.length;

        for( var i = 0; i<len; i++){
            var id = effects[i]['n'];
            var name = effects[i]['name'];
            $("#sel_effect").append("<option value='"+id+"'>"+name+"</option>");
        }
    });
}


function init()
{
    console.log("init");
    populate_effects();
}