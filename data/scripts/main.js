"use strict";

function set_effect(id) 
{
    console.log("set_effect");
    console.log(id);
    $.get("api/seteffect", 
    { "id" : id }).done(
        function(data) { 
            populate_properties(data)          // if successful this will give us the properties
        }          
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

function set_prop(from) 
{
    console.log(from);
    $.get("api/setprop",
        { "effect": 1,
        "propid": from.id,
        "value": from.value }).done(function (data){
            console.log(data);
        });
}

// create control for a property element
function appendControl(parent, element)
{
    if (element.type == 1)  {       // integer
        parent.append(`${element.name}: <input type="range" min="0" max="${element.range}" value="${element.value}" id="${element.n}" onchange="set_prop(this)"></input>`);
    } else if (element.type == 2) {
        parent.append(`${element.name}: <input type="color" value="${element.value}" id="${element.n}" oninput="set_prop(this)"></input>`);
    }
}

function populate_properties(data)
{
    $("#properties").empty();
    data['props'].forEach(element => { appendControl($("#properties"), element); 
                                       $("#properties").append("<p>");
                                    });
}

function get_effect_properties()
{
    $.get("api/geteffectprops").done(function(data) {
        populate_properties(data);
    });
}


function init()
{
    console.log("init");
    populate_effects();
    get_effect_properties();
}