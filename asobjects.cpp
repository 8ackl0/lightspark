/**************************************************************************
    Lighspark, a free flash player implementation

    Copyright (C) 2009  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <list>
#include <algorithm>

#include "asobjects.h"
#include "swf.h"

using namespace std;

extern __thread SystemState* sys;

ASStage::ASStage():width(640),height(480)
{
	setVariableByName("width",SWFObject(&width,true));
	setVariableByName("height",SWFObject(&height,true));
}

void ASArray::_register()
{
	setVariableByName("constructor",SWFObject(new Function(constructor),true));
}

SWFObject ASArray::constructor(const SWFObject& th, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called Array constructor");
	return SWFObject();
}

ASMovieClipLoader::ASMovieClipLoader()
{
	_register();
}

void ASMovieClipLoader::_register()
{
	setVariableByName("constructor",SWFObject(new Function(constructor),true));
	setVariableByName("addListener",SWFObject(new Function(addListener),true));
}

SWFObject ASMovieClipLoader::constructor(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called MovieClipLoader constructor");
	return SWFObject();
}

SWFObject ASMovieClipLoader::addListener(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called MovieClipLoader::addListener");
	return SWFObject();
}

ASXML::ASXML()
{
	_register();
}

void ASXML::_register()
{
	setVariableByName("constructor",SWFObject(new Function(constructor),true));
}

SWFObject ASXML::constructor(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called XML constructor");
	return SWFObject();
}

void ASObject::_register()
{
	setVariableByName("constructor",SWFObject(new Function(constructor),true));
}

SWFObject ASObject::constructor(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called Object constructor");
	return SWFObject();
}

ASString::ASString(const STRING& s)
{
	data.reserve(s.String.size());
	for(int i=0;i<s.String.size();i++)
		data.push_back(s.String[i]);
}

STRING ASString::toString()
{
	return STRING(data.data());
}

ASMovieClip::ASMovieClip():_visible(1),_width(100)
{
	sem_init(&sem_frames,0,1);
	_register();
}

bool ASMovieClip::list_orderer(const IDisplayListElem* a, int d)
{
	return a->getDepth()<d;
}

void ASMovieClip::addToDisplayList(IDisplayListElem* t)
{
	list<IDisplayListElem*>::iterator it=lower_bound(displayList.begin(),displayList.end(),t->getDepth(),list_orderer);
	displayList.insert(it,t);
}

SWFObject ASMovieClip::createEmptyMovieClip(const SWFObject& obj, arguments* args)
{
	ASMovieClip* th=dynamic_cast<ASMovieClip*>(obj.getData());
	if(th==NULL)
		LOG(ERROR,"Not a valid ASMovieClip");

	LOG(CALLS,"Called createEmptyMovieClip: " << args->args[0]->toString() << " " << args->args[1]->toString());
	ASMovieClip* ret=new ASMovieClip();

	IDisplayListElem* t=new ASObjectWrapper(ret,args->args[1]->toInt());
	list<IDisplayListElem*>::iterator it=lower_bound(th->dynamicDisplayList.begin(),th->dynamicDisplayList.end(),t->getDepth(),list_orderer);
	th->dynamicDisplayList.insert(it,t);

	SWFObject r(ret,true);
	th->setVariableByName(args->args[0]->toString(),r);
	return r;
}

SWFObject ASMovieClip::moveTo(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called moveTo");
	return SWFObject();
}

SWFObject ASMovieClip::lineTo(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called lineTo");
	return SWFObject();
}

SWFObject ASMovieClip::lineStyle(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called lineStyle");
	return SWFObject();
}

SWFObject ASMovieClip::swapDepths(const SWFObject&, arguments* args)
{
	LOG(NOT_IMPLEMENTED,"Called swapDepths");
	return SWFObject();
}

void ASMovieClip::_register()
{
	setVariableByName("_visible",SWFObject(&_visible,true));
	setVariableByName("_width",SWFObject(&_width,true));
	setVariableByName("swapDepths",SWFObject(new Function(swapDepths),true));
	setVariableByName("lineStyle",SWFObject(new Function(lineStyle),true));
	setVariableByName("lineTo",SWFObject(new Function(lineTo),true));
	setVariableByName("moveTo",SWFObject(new Function(moveTo),true));
	setVariableByName("createEmptyMovieClip",SWFObject(new Function(createEmptyMovieClip),true));
}

void ASMovieClip::Render()
{
	parent=sys->currentClip;
	ASMovieClip* clip_bak=sys->currentClip;
	sys->currentClip=this;

	LOG(TRACE,"Render MovieClip");
	state.next_FP=min(state.FP+1,frames.size()-1);

	list<Frame>::iterator frame=frames.begin();
	for(int i=0;i<state.FP;i++)
		frame++;
	frame->Render(0);

	//Render objects added at runtime;
	list<IDisplayListElem*>::iterator it=dynamicDisplayList.begin();
	for(it;it!=dynamicDisplayList.end();it++)
		(*it)->Render();

	if(state.FP!=state.next_FP)
	{
		state.FP=state.next_FP;
		sys->setUpdateRequest(true);
	}
	LOG(TRACE,"End Render MovieClip");

	sys->currentClip=clip_bak;
}

