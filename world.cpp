/*
  Puzilla
  Copyright (C) 2013-2019 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "world.h"
#include <../Opt/chipmunk/chipmunk.h>

sWorld World;
static cpSpace *space;
static unsigned int timeNextBlock = 0;
static ticks_t WorldTicks;

#define COLLISIONTYPE_FIELD       0
#define COLLISIONTYPE_PLAYER      1
#define COLLISIONTYPE_BLOCK       2
#define CATEGORY_COLLIDINGSTUFF   (1<<0)
#define CATEGORY_BLOCKS           (1<<1)
#define COLLIDEMASK_PLAYER        (CATEGORY_COLLIDINGSTUFF | CATEGORY_BLOCKS)
#define COLLIDEMASK_FIELD         (CATEGORY_COLLIDINGSTUFF | CATEGORY_BLOCKS)
#define COLLIDEMASK_BLOCK         (CATEGORY_COLLIDINGSTUFF | CATEGORY_BLOCKS)

#define BLOCK_WIDTH s(10)
#define FIELDX 11
#define FIELDY 15
#define FIELD_WIDTH (BLOCK_WIDTH*FIELDX)
#define FIELD_HEIGHT (BLOCK_WIDTH*FIELDY)
#define BOX_SCALE (BLOCK_WIDTH/s(64))
#define CHAR_SCALE (s(1.5)*BLOCK_WIDTH/s(64))
#define CHAR_SHIFT (s(6)*CHAR_SCALE)
#define CHAR_RADIUS (BLOCK_WIDTH*s(0.25))
#define CHAR_FIREX (BLOCK_WIDTH*s(0.4))
#define CHAR_FIREY (BLOCK_WIDTH*s(0.15))

#define PLAYER_VELOCITY 40.0f

#define PLAYER_GROUND_ACCEL_TIME 0.02f
#define PLAYER_GROUND_ACCEL (PLAYER_VELOCITY/PLAYER_GROUND_ACCEL_TIME)

#define PLAYER_AIR_ACCEL_TIME 0.2f
#define PLAYER_AIR_ACCEL (PLAYER_VELOCITY/PLAYER_AIR_ACCEL_TIME)

#define JUMP_HEIGHT 6.0f
#define JUMP_BOOST_HEIGHT 12.0f
#define FALL_VELOCITY 50.0f
#define GRAVITY 135.0f

ZL_Surface srfBoxBG, srfBoxOuter, srfBoxInner, srfChar, srfLink;

struct sBlock;
vector<sBlock*> Blocks;

struct sBlock
{
	cpBody *body;
	enum eColorType { C_RED, C_GREEN, C_YELLOW, C_BLUE } color_type;
	ZL_Color color, colorglow;
	scalar texscale;
	ticks_t destroy_from, destroy_to, collisionsound_time;
	int linknum;
	sBlock* Links[10];
	sBlock(int x, bool starts_at_bottom = false)
	{
		body = cpSpaceAddBody(space, cpBodyNew(50, cpMomentForBox(50, BLOCK_WIDTH*s(0.99), BLOCK_WIDTH*s(0.99))));
		cpBodySetUserData(body, this);
		cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, BLOCK_WIDTH*s(0.99), BLOCK_WIDTH*s(0.99), 0));
		cpShapeSetCollisionType(shape, COLLISIONTYPE_BLOCK);
		cpShapeSetFilter(shape, cpShapeFilterNew(CP_NO_GROUP, CATEGORY_BLOCKS, COLLIDEMASK_BLOCK));
		cpShapeSetElasticity(shape, 0.01f);
		cpShapeSetFriction(shape, 1);
		cpShapeSetUserData(shape, this);
		if (starts_at_bottom)
		{
			cpBodySetPosition(body, cpv(BLOCK_WIDTH*x + BLOCK_WIDTH/s(2), BLOCK_WIDTH*s(0.6)));
		}
		else
		{
			cpBodySetPosition(body, cpv(BLOCK_WIDTH*x + BLOCK_WIDTH/s(2), FIELD_HEIGHT - BLOCK_WIDTH*s(0.5)));
			cpBodySetVelocity(body, cpv(0, -50));
		}
		body->velocity_func = &UpdateVelocityFunc;
		//TODO: cpBodySetVelLimit(body, 50);
		//apply body pos to shape so other boxes can correctly query for it if called before updating cpSpace
		cpShapeUpdate(shape, body->transform);
		texscale = BOX_SCALE;
		destroy_from = destroy_to = 0;
		collisionsound_time = ZLTICKS;
		eColorType MaxColorType = (World.level < 10 ? C_YELLOW : C_BLUE);
		color_type = (eColorType)RAND_INT_RANGE(C_RED, MaxColorType);
		switch (color_type)
		{
			case C_RED:    color = ZLRGB(.7,0 ,0 ); colorglow = ZLRGB(1 , .2,0 ); break;
			case C_GREEN:  color = ZLRGB(0 ,.7,0 ); colorglow = ZLRGB(.2, 1 ,0 ); break;
			case C_YELLOW: color = ZLRGB(.7,.7,0 ); colorglow = ZLRGB(1 , 1 ,.2); break;
			case C_BLUE:   color = ZLRGB(0 ,.7,.7); colorglow = ZLRGB(.2, 1 ,1 ); break;
		}
		linknum = 0;;
	}
	~sBlock()
	{
		cpShape *s = body->shapeList;
		cpShapeSetUserData(s, NULL);
		cpSpaceRemoveShape(space, s);
		cpSpaceRemoveBody(space, body);
		cpShapeDestroy(s);
		cpBodyDestroy(body);
	}
	bool UpdateShouldErase()
	{
		if (destroy_to)
		{
			if (WorldTicks >= destroy_to) return true;
			scalar scale = s(destroy_to - WorldTicks)/s(destroy_to - destroy_from);
			texscale = BOX_SCALE*scale;

			cpFloat sz = MAX(0.01f, BLOCK_WIDTH*0.5f*scale);
			cpVect verts[4] = { { -sz, -sz }, { -sz,  sz }, { sz,  sz }, { sz, -sz } };
			cpPolyShapeSetVerts(body->shapeList, 4, verts, cpTransformIdentity);
		}
		else if (body->p.y < 0) return true; //somehow fell out of the play field...
		else
		{
			linknum = 0;
			static const cpFloat r = BLOCK_WIDTH*s(0.8);
			for (int i = 0; i <= 8; i++)
			{
				if (i == 4) i++;
				cpVect pnt = cpv( body->p.x + r * ((i%3)-1) , body->p.y + r * ((i/3)-1) );
				cpShape *s = cpSpacePointQueryNearest(space, pnt, 0, cpShapeFilterNew(CP_NO_GROUP, CATEGORY_BLOCKS, CATEGORY_BLOCKS), 0);
				if (s && s->userData) AddLink((sBlock*)s->userData);
			}
			CP_BODY_FOREACH_ARBITER(body, arb)
			{
				if (linknum == 10) break;
				if (arb->a->type != COLLISIONTYPE_BLOCK || arb->b->type != COLLISIONTYPE_BLOCK) continue;
				AddLink((sBlock*)(arb->body_a == body ? arb->b : arb->a)->userData);
			}
		}
		return false;
	}
	void AddLink(sBlock* b)
	{
		if (b == this || linknum == 10 || b->destroy_to || b->color_type != color_type) return;
		for (int iLink = 0; iLink < linknum; iLink++) if (Links[iLink] == b) return;
		Links[linknum++] = b;
	}
	void RemoveLink(sBlock* b)
	{
		int n = 0;
		for (; n < linknum; n++) if (Links[n] == b) break;
		if (n == linknum) return;
		linknum--;
		for (; n < linknum; n++) Links[n] = Links[n+1];
	}
	void Draw()
	{
		//cpVect *v = ((cpPolyShape*)body->shapeList)->verts;
		//ZL_Display::PushMatrix();
		//ZL_Display::Transform(body->p.x, body->p.y, body->rot.x, body->rot.y);
		//ZL_Display::DrawQuad(v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y, v[3].x, v[3].y, ZL_Color::Red, ZL_Color::Blue);
		////ZL_Display::DrawRect(bb.l, bb.b, bb.r, bb.t, ZL_Color::Red, ZL_Color::Blue);
		//ZL_Display::PopMatrix();

		srfBoxInner.Draw(body->p, body->a, texscale, texscale, (linknum ? colorglow : color));
		srfBoxOuter.Draw(body->p, body->a, texscale, texscale);

		/*
		cpFloat r = BLOCK_WIDTH*s(0.7);
		cpShape* s[4];
		s[0] = cpSpacePointQueryFirst(space, cpv(body->p.x - r, body->p.y), LAYER_BLOCKS, 0);
		s[1] = cpSpacePointQueryFirst(space, cpv(body->p.x + r, body->p.y), LAYER_BLOCKS, 0);
		s[2] = cpSpacePointQueryFirst(space, cpv(body->p.x, body->p.y - r), LAYER_BLOCKS, 0);
		s[3] = cpSpacePointQueryFirst(space, cpv(body->p.x, body->p.y + r), LAYER_BLOCKS, 0);
		if (s[0] && ((sBlock*)s[0]->userData)->color_type == color_type) ZL_Display::DrawLine(body->p, s[0]->body->p, color);
		if (s[1] && ((sBlock*)s[1]->userData)->color_type == color_type) ZL_Display::DrawLine(body->p, s[1]->body->p, color);
		if (s[2] && ((sBlock*)s[2]->userData)->color_type == color_type) ZL_Display::DrawLine(body->p, s[2]->body->p, color);
		if (s[3] && ((sBlock*)s[3]->userData)->color_type == color_type) ZL_Display::DrawLine(body->p, s[3]->body->p, color);
		CP_BODY_FOREACH_ARBITER(body, arb)
		{
			if (arb->a->collision_type == COLLISION_BLOCK && arb->b->collision_type == COLLISION_BLOCK && ((sBlock*)(arb->body_a == body ? arb->b : arb->a)->userData)->color_type == color_type) ZL_Display::DrawLine(body->p, (arb->body_a == body ? arb->body_b : arb->body_a)->p, color);
		}
		*/

		/*
		cpFloat r = BLOCK_WIDTH*s(0.8);
		for (int i = 0; i <= 8; i++)
		{
			if (i == 4) i++;
			cpVect pnt = cpv( body->p.x + r * ((i%3)-1) , body->p.y + r * ((i/3)-1) );
			cpShape *s = cpSpacePointQueryFirst(space, pnt, LAYER_BLOCKS, 0);
			if (s && ((sBlock*)s->userData)->color_type == color_type)
				ZL_Display::DrawLine(body->p, s->body->p, color);
		}
		CP_BODY_FOREACH_ARBITER(body, arb)
		{
			if (arb->a->collision_type != COLLISION_BLOCK || arb->b->collision_type != COLLISION_BLOCK) continue;
			cpShape *s = (arb->body_a == body ? arb->b : arb->a);
			if (((sBlock*)s->userData)->color_type == color_type)
				ZL_Display::DrawLine(body->p, s->body->p, color);
		}
		*/

		//cpSpaceBBQuery(space, cpBBNew(body->p.x - r, body->p.y - r, body->p.x + r, body->p.y + r), LAYER_BLOCKS, 0, (cpSpaceBBQueryFunc)&funcQueryNeighborBlocks, this);
		//ZL_Display::DrawCircle(body->p, r, ZL_Color::Red);
	}

	void DrawLinks()
	{
		for (int iLink = 0; iLink < linknum; iLink++)
		{
			if (this < Links[iLink])
			{
				//ZL_Display::DrawLine(body->p, (*pLink)->body->p, color);
				ZL_Vector p = body->p;
				ZL_Vector v = ZL_Vector(Links[iLink]->body->p) - p;
				scalar vlen = v.GetLength();
				v /= vlen;
				p -= v;
				vlen += 2;
				scalar pulse = (sin(s(WorldTicks)/s(500))+s(2))/s(3);
				srfLink.Draw(p, v.GetAngle(), vlen/s(srfLink.GetWidth()), s(.05)+(s(0.1)*pulse), colorglow);
			}
		}
	}

	void FillChain(vector<sBlock*>& chain)
	{
		chain.push_back(this);
		for (int iLink = 0; iLink < linknum; iLink++)
			if (find(chain.begin(), chain.end(), Links[iLink]) == chain.end())
				Links[iLink]->FillChain(chain);
		/*
		cpFloat r = BLOCK_WIDTH*s(0.8);
		for (int i = 0; i <= 8; i++)
		{
			if (i == 4) i++;
			cpVect pnt = cpv( body->p.x + r * ((i%3)-1) , body->p.y + r * ((i/3)-1) );
			cpShape *s = cpSpacePointQueryFirst(space, pnt, LAYER_BLOCKS, 0);
			if (s && ((sBlock*)s->userData)->color_type == color_type && find(chain.begin(), chain.end(), ((sBlock*)s->userData)) == chain.end())
				((sBlock*)s->userData)->FillChain(chain);
		}
		CP_BODY_FOREACH_ARBITER(body, arb)
		{
			if (arb->a->collision_type != COLLISION_BLOCK || arb->b->collision_type != COLLISION_BLOCK) continue;
			cpShape *s = (arb->body_a == body ? arb->b : arb->a);
			if (((sBlock*)s->userData)->color_type == color_type && find(chain.begin(), chain.end(), ((sBlock*)s->userData)) == chain.end())
				((sBlock*)s->userData)->FillChain(chain);
		}
		*/
	}

	//static void funcQueryNeighborBlocks(cpShape *shape, sBlock *self)
	//{
	//	if (shape == self->body->shapeList || cpShapeGetCollisionType(shape) != COLLISION_BLOCK) return;
	//	if (((sBlock*)shape->userData)->color_type != self->color_type) return;
	//	ZL_Display::DrawLine(self->body->p, shape->body->p, self->color);
	//}

	static cpBool CollisionBlock2Anything(cpArbiter *arb, cpSpace *space, void*)
	{
		//if (!cpArbiterIsFirstContact(arb)) return;
		CP_ARBITER_GET_BODIES(arb, b1, b2);
		if ((ZLTICKS-((sBlock*)b1->userData)->collisionsound_time) < 150) return cpTrue;
		cpFloat vlen = cpvlength(b1->v); //cpvlength(cpArbiterTotalImpulse(arb));
		if (vlen < 5) return cpTrue;
		//extern vector<sBlock*> Blocks; ZL_LOG2("BLOCK", "[%d] COLL VLEN: %f", find(Blocks.begin(), Blocks.end(), (sBlock*)b1->userData) - (Blocks.begin()), vlen);
		((sBlock*)b1->userData)->collisionsound_time = ZLTICKS;
		int vol = MIN(100, (int)(vlen*2.0f));
		imcBlock.SetChannelVolume(imcBlockCounter, vol);
		imcBlock.NoteOn(imcBlockCounter, 60-(vol/12)-RAND_INT_RANGE(0,3));
		imcBlockCounter = (imcBlockCounter + 1) % 8;
		return cpTrue;
	}
	static void UpdateVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
	{
		cpBodyUpdateVelocity(body, gravity, damping, dt);
		body->v = cpvclamp(body->v, 50);
	}
};

struct sPlayer
{
	cpBody *body;
	cpShape *shape, *shapehead;
	cpFloat remainingBoost;
	bool grounded;
	bool lastJumpState;
	float chargeAmount;
	bool lookLeft;

	sPlayer()
	{
		//body = cpSpaceAddBody(space, cpBodyNew(5, cpMomentForBox(5, BLOCK_WIDTH/2, BLOCK_WIDTH)));
		//cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body, BLOCK_WIDTH/2, BLOCK_WIDTH-s(BLOCK_WIDTH/s(100))));
		body = cpSpaceAddBody(space, cpBodyNew(1.f, INFINITY)); //cpMomentForCircle(1, 0, BLOCK_WIDTH/4, cpvzero)));
		shape =     cpSpaceAddShape(space, cpCircleShapeNew(body, CHAR_RADIUS, cpv(0, 0)));
		shapehead = cpSpaceAddShape(space, cpCircleShapeNew(body, BLOCK_WIDTH*s(0.18), cpv(0, BLOCK_WIDTH*s(0.3))));
		cpBodySetUserData(body, this);
		cpShapeSetCollisionType(shape, COLLISIONTYPE_PLAYER);
		cpShapeSetFilter(shape, cpShapeFilterNew(CP_NO_GROUP, CATEGORY_COLLIDINGSTUFF, COLLIDEMASK_PLAYER));
		cpShapeSetElasticity(shape, 0);
		cpShapeSetFriction(shape, 0);
		cpShapeSetUserData(shape, this);
		cpShapeSetCollisionType(shapehead, COLLISIONTYPE_PLAYER);
		cpShapeSetFilter(shapehead, cpShapeFilterNew(CP_NO_GROUP, CATEGORY_COLLIDINGSTUFF, COLLIDEMASK_PLAYER));
		cpShapeSetElasticity(shapehead, 0);
		cpShapeSetFriction(shapehead, 0);
		cpShapeSetUserData(shapehead, this);
		body->velocity_func = &UpdateVelocityFunc;
		cpBodySetPosition(body, cpv(FIELD_WIDTH/s(2), FIELD_HEIGHT - BLOCK_WIDTH/s(2)));
		//TODO: //cpBodySetVelLimit(body, 100);
		//TODO: //cpBodySetAngVelLimit(body, 0);
		//cpBodySetVel(body, cpv(0, -50));
		Reset();
	}
	void Reset()
	{
		remainingBoost = 0;
		grounded = false;
		lastJumpState = false;
		chargeAmount = 0;
		lookLeft = false;
	}
	void Draw()
	{
		//cpVect *v = ((cpPolyShape*)body->shapeList)->verts;
		//ZL_Display::PushMatrix();
		//ZL_Display::Transform(body->p.x, body->p.y, body->rot.x, body->rot.y);
		//ZL_Display::DrawQuad(v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y, v[3].x, v[3].y, ZL_Color::Green, ZL_Color::Orange);
		////ZL_Display::DrawRect(bb.l, bb.b, bb.r, bb.t, ZL_Color::Red, ZL_Color::Blue);
		//ZL_Display::PopMatrix();
		srfChar.FlipH();
		srfChar.Draw(body->p.x + (lookLeft ? -CHAR_SHIFT : CHAR_SHIFT), body->p.y - (BLOCK_WIDTH/s(3)), (lookLeft ? -CHAR_SCALE : CHAR_SCALE), CHAR_SCALE);
		//ZL_Display::DrawCircle(((cpCircleShape*)shape)->tc,     ((cpCircleShape*)shape)->r,     ZL_Color::Green, ZLRGBA(1,1,0,.3));
		//ZL_Display::DrawCircle(((cpCircleShape*)shapehead)->tc, ((cpCircleShape*)shapehead)->r, ZL_Color::Green, ZLRGBA(1,1,0,.3));
		//ZL_Display::DrawLine(body->p, cpvadd(body->p, cpvmult(body->rot, ((cpCircleShape*)body->shapeList)->r*(lookLeft?-2:2))), ZL_Color::Green);
		if (chargeAmount)
		{
			ZL_Display::DrawCircle(body->p.x + (lookLeft ? -CHAR_FIREX : CHAR_FIREX), body->p.y + CHAR_FIREY, chargeAmount*0.03f, (chargeAmount >= 100 ? ZL_Color::Yellow : ZL_Color::Blue), (chargeAmount >= 100 ? ZL_Color::Orange : ZL_Color::Cyan));
		}
	}
	void Calculate()
	{
		if (World.PlayerMoveInput.x)
		{
			lookLeft = (World.PlayerMoveInput.x < 0);
			if (grounded) srfChar.SetTilesetIndex(3 + ((WorldTicks/100)%3));
		}
		if (World.PlayerAttackInput)
		{
			if (chargeAmount == 0) imcCharge.Play(true);
			if (chargeAmount < 100)
			{
				chargeAmount += ZLELAPSEDF(100);
				if (chargeAmount >= 100) { imcFull.Play(true); chargeAmount = 100; }
			}
			//chargeAmount = 0;
			//World.PlayerAttackInput = false;
		}
		if (!World.PlayerAttackInput && chargeAmount)
		{
			if (chargeAmount >= 100)
			{
				imcDischarge.Play(true);
				ZL_LOG1("PLAYER", "Discharge at %f", chargeAmount);
				cpVect pp = cpv(body->p.x + (lookLeft ? -CHAR_FIREX : CHAR_FIREX), body->p.y + CHAR_FIREY);
				cpShape *s = cpSpacePointQueryNearest(space, pp, 100.0f*0.03f, cpShapeFilterNew(0, CATEGORY_BLOCKS, CATEGORY_BLOCKS), NULL);
				if (s && s->userData)
				{
					vector<sBlock*> chain;
					((sBlock*)s->userData)->FillChain(chain);
					for (vector<sBlock*>::iterator it = chain.begin(); it != chain.end(); ++it)
					{
						(*it)->destroy_from = WorldTicks;
						(*it)->destroy_to = WorldTicks + RAND_INT_RANGE(300, 500);
					}
					unsigned int n = (unsigned int)chain.size();
					World.combotime = ZLTICKS;
					World.combocount = n;
					World.score += 100*(int)((s(n+5)*(s(n)/s(4)))/s(1.5));
					World.level++;
					for (n = MIN(5, 1+(World.combocount/4)); n--;)
					{
						//imcExplosion.SetChannelVolume(imcExplosionCounter, vol);
						imcExplosion.NoteOn(imcExplosionCounter, 72+RAND_INT_RANGE(0,10));
						imcExplosionCounter = (imcExplosionCounter + 1) % 8;
					}
				}
			}
			else
			{
				imcPush.SetSongVolume((int)chargeAmount);
				imcPush.Play(true);
				ZL_LOG1("PLAYER", "PushCharge at %f", chargeAmount);
				cpVect pp = cpv(body->p.x + (lookLeft ? -CHAR_FIREX : CHAR_FIREX), body->p.y + CHAR_FIREY);
				cpFloat maxdist = (chargeAmount*0.15f+BLOCK_WIDTH);
				cpFloat maxdistsq = maxdist*maxdist;
				for (vector<sBlock*>::iterator it = Blocks.begin(); it != Blocks.end(); ++it)
				{
					cpFloat distsq = cpvdistsq(pp, (*it)->body->p);
					if (distsq < maxdistsq)
					{
						//cpNearestPointQueryInfo npq;
						//cpShapeNearestPointQuery((*it)->body->shapeList, pp, &npq);
						cpVect off = cpvsub(pp, (*it)->body->p);
						//cpVect dir = cpvmult(cpvnormalize_safe(cpvsub(npq.p, pp)), 20*(maxdist - distsq /*npq.d*/));
						cpVect dir = cpvmult(cpvnormalize(cpvneg(off)), 400*(maxdist - cpfsqrt(distsq) /*npq.d*/));
						if (dir.y < -cpfabs(dir.x)) { dir.x *= 0.1f; dir.y *= 0.1f; }
						ZL_LOG4("BLOCK", "Applying V: %f - %f , AT OFFSET: %f - %f", dir.x, dir.y, off.x, off.y);

						cpBodyApplyImpulseAtWorldPoint((*it)->body, dir, pp);
					}
				}
			}
			chargeAmount = 0;
			imcCharge.Stop();
		}
	}

	void UpdateVelocity(const cpVect& gravity, cpFloat damping, cpFloat dt)
	{
		// Grab the grounding normal from last frame
		bool is_grounded = false, is_headbumping = false;
		CP_BODY_FOREACH_ARBITER(body, arb)
		{
			arb->swapped = (body == arb->body_b);
			cpFloat ny = cpArbiterGetNormal(arb).y;
			if (ny < -0.5) is_grounded = true;
			else if (ny > 0.5) is_headbumping = true;
		}
		if (!grounded && is_grounded)
		{
			srfChar.SetTilesetIndex(0);
			//printf("LAND\n");
		}
		else if (grounded && !is_grounded)
		{
			srfChar.SetTilesetIndex(1);
			//printf("AIR\n");
		}
		grounded = is_grounded;

		bool jumpState = (World.PlayerMoveInput.y > 0.0f);
		if (jumpState && !lastJumpState && grounded)
		{
			//printf("JUMP\n");
			body->v.y = cpfsqrt(2.0*JUMP_HEIGHT*GRAVITY);
			remainingBoost = JUMP_BOOST_HEIGHT/body->v.y;
			CP_BODY_FOREACH_ARBITER(body, arb) { arb->surface_vr = cpvzero; arb->u = 0; }
			lastJumpState = true;
		}
		if (lastJumpState == true && jumpState == false) lastJumpState = false;

		// Do a normal-ish update
		if (remainingBoost > 0.0f)
		{
			remainingBoost -= dt;
			if (!jumpState || is_headbumping || remainingBoost < 0) remainingBoost = 0;
		}
		cpBodyUpdateVelocity(body, (remainingBoost ? cpvzero : gravity), damping, dt);
		if (body->p.y < CHAR_RADIUS) { body->p.y = CHAR_RADIUS+(BLOCK_WIDTH/s(10)); grounded = true; }
		if (body->p.x < CHAR_RADIUS) { body->p.x = CHAR_RADIUS+(BLOCK_WIDTH/s(10)); grounded = true; }
		if (body->p.x > FIELD_WIDTH-CHAR_RADIUS) { body->p.x = FIELD_WIDTH-CHAR_RADIUS-(BLOCK_WIDTH/s(10)); grounded = true; }

		// Target horizontal speed for air/ground control
		cpFloat target_vx = PLAYER_VELOCITY*World.PlayerMoveInput.x;

		// Update the surface velocity and friction
		shapehead->surfaceV = shape->surfaceV = (grounded ? cpv(target_vx, 0.0) : cpvzero);
		shapehead->u = shape->u = (grounded ? PLAYER_GROUND_ACCEL/GRAVITY*0.1f : 0.0f);

		// Apply air control if not grounded
		if(!grounded)
		{
			// Smoothly accelerate the velocity
			body->v.x = cpflerpconst(body->v.x, target_vx, PLAYER_AIR_ACCEL*dt);
		}
		else if (shape->surfaceV.x)
		{
			body->v.x = cpflerpconst(body->v.x, shape->surfaceV.x, PLAYER_GROUND_ACCEL*dt);
		}
		//body->v.y = cpfclamp(body->v.y, -FALL_VELOCITY, INFINITY);

		body->v = cpvclamp(body->v, 100);
		body->w = 0;
	}
	static void UpdateVelocityFunc(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
	{
		((sPlayer*)body->userData)->UpdateVelocity(gravity, damping, dt);
	}
} *Player;

void sWorld::InitGlobal()
{
	srfBoxOuter = ZL_Surface("Data/box_outer.png").SetDrawOrigin(ZL_Origin::Center);
	srfBoxInner = ZL_Surface("Data/box_inner.png").SetDrawOrigin(ZL_Origin::Center);
	srfBoxBG = ZL_Surface("Data/box_inner.png").SetScale(BOX_SCALE).SetTextureRepeatMode();
	srfChar = ZL_Surface("Data/char.png").SetTilesetClipping(3, 2).SetDrawOrigin(ZL_Origin::BottomCenter).SetScale(CHAR_SCALE);
	srfLink = ZL_Surface("Data/link.png").SetDrawOrigin(ZL_Origin::CenterLeft).SetRotateOrigin(ZL_Origin::CenterLeft);

	space = cpSpaceNew();
	//cpSpaceSetIterations(space, 2);
	//cpSpaceSetSleepTimeThreshold(space, 1);
	cpSpaceSetGravity(space, cpv(0.0f, -GRAVITY));
	//cpSpaceSetDamping(space, s(0.9));
	cpSpaceAddCollisionHandler(space, COLLISIONTYPE_BLOCK, COLLISIONTYPE_FIELD)->beginFunc = sBlock::CollisionBlock2Anything;
	cpSpaceAddCollisionHandler(space, COLLISIONTYPE_BLOCK, COLLISIONTYPE_PLAYER)->beginFunc = sBlock::CollisionBlock2Anything;
	cpSpaceAddCollisionHandler(space, COLLISIONTYPE_BLOCK, COLLISIONTYPE_BLOCK)->beginFunc = sBlock::CollisionBlock2Anything;
	//cpSpaceAddCollisionHandler(space, COLLISION_PLAYER, COLLISION_ENEMY)->beginFunc = CollisionPlayerEnemy;

	cpShape *s[3];
	s[0] = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(0, 0), cpv(FIELD_WIDTH, 0), 1));
	s[1] = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(0, 0), cpv(0, FIELD_HEIGHT), 1));
	s[2] = cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(FIELD_WIDTH, 0), cpv(FIELD_WIDTH, FIELD_HEIGHT), 1));
	cpShapeSetFilter(s[0], cpShapeFilterNew(CP_NO_GROUP, CATEGORY_COLLIDINGSTUFF, COLLIDEMASK_FIELD)); cpShapeSetFriction(s[0], 1); cpShapeSetCollisionType(s[0], COLLISIONTYPE_FIELD);
	cpShapeSetFilter(s[1], cpShapeFilterNew(CP_NO_GROUP, CATEGORY_COLLIDINGSTUFF, COLLIDEMASK_FIELD)); cpShapeSetFriction(s[1], 1); cpShapeSetCollisionType(s[1], COLLISIONTYPE_FIELD);
	cpShapeSetFilter(s[2], cpShapeFilterNew(CP_NO_GROUP, CATEGORY_COLLIDINGSTUFF, COLLIDEMASK_FIELD)); cpShapeSetFriction(s[2], 1); cpShapeSetCollisionType(s[2], COLLISIONTYPE_FIELD);
	cpShapeSetFriction(s[1], 0);
	cpShapeSetFriction(s[2], 0);

	Player = new sPlayer();
}

void sWorld::Init()
{
	WorldTicks = 0;
	gameovertime = 0;
	score = 0;
	combotime = combocount = level = 0;
	//level = 200; //test
	for (vector<sBlock*>::iterator it = Blocks.begin(); it != Blocks.end(); ++it) delete *it;
	Blocks.clear();
	for (int x = 0; x < FIELDX; x++) Blocks.push_back(new sBlock(x, true));
	Player->Reset();
	cpBodySetPosition(Player->body, cpv(BLOCK_WIDTH*FIELDX/2, BLOCK_WIDTH + BLOCK_WIDTH*s(0.4)));
	//gameovertime = 1;return;
	for (int i = 0; i < 50; i++) cpSpaceStep(space, 0.016f); //simulate a bit
	timeNextBlock = WorldTicks + 3000;
}

void sWorld::Calculate()
{
	if (gameovertime) return;
	WorldTicks += ZLELAPSEDTICKS;

	//cpBodySetAngle(Player->body, Player->body->a + ZL_Math::RelAngle(Player->body->a, 0)*ZLELAPSEDF(8));
	//if      (PlayerMoveInput.x > 0) { /*Player->body->p.x += ZLELAPSEDF(10);*/ Player->body->shapeList->surfaceV.x =  20; }
	//else if (PlayerMoveInput.x < 0) { /*Player->body->p.x -= ZLELAPSEDF(10);*/ Player->body->shapeList->surfaceV.x = -20; }
	//else Player->body->shapeList->surfaceV.x = 0;

	//Calculate space before updating blocks
	//cpSpaceStep(space, ZLELAPSED);
	static ticks_t TICKSUM = 0;
	static const cpFloat dp = s(16.0/1000.0);
	TICKSUM += ZLELAPSEDTICKS;
	while (TICKSUM > 16)
	{
		cpSpaceStep(space, dp);
		TICKSUM -= 16;
	}

	if (timeNextBlock <= WorldTicks)
	{
		timeNextBlock += (2000 - MIN(1700, (level*10)));
		Blocks.push_back(new sBlock(RAND_INT_RANGE(0,FIELDX-1)));
	}

	//UpdateShouldErase uses cpSpacePointQueryFirst so new blocks need at least one cpSpaceStep before doing this (not anymore because of cpShapeUpdate in sBlock::sBlock())
	for (vector<sBlock*>::iterator it = Blocks.begin(); it != Blocks.end();)
	{
		if ((*it)->body->p.y > FIELD_HEIGHT && (*it)->body->p.x > FIELD_WIDTH/3 && (*it)->body->p.x < FIELD_WIDTH*2/3)
		{
			//set gameover state
			if (Player->chargeAmount) imcCharge.Stop();
			gameovertime = ZLTICKS;
			return;
		}
		if ((*it)->UpdateShouldErase())
		{
			for (vector<sBlock*>::iterator itLinks = Blocks.begin(); itLinks != Blocks.end(); ++itLinks) (*itLinks)->RemoveLink(*it);
			delete *it;
			it = Blocks.erase(it);
		}
		else ++it;
	}

	Player->Calculate();
}

void sWorld::Draw()
{
	ZL_Display::ClearFill(ZL_Color::Black);
	scalar borderw = ((ZLWIDTH/ZLHEIGHT*(FIELD_HEIGHT+BLOCK_WIDTH+BLOCK_WIDTH))-FIELD_WIDTH)/s(2);
	ZL_Display::PushOrtho(0 - borderw, FIELD_WIDTH + borderw, -BLOCK_WIDTH, BLOCK_WIDTH+FIELD_HEIGHT);
	srfBoxBG.DrawTo(-BLOCK_WIDTH*100, -BLOCK_WIDTH*100, BLOCK_WIDTH*1000, BLOCK_WIDTH*1000, ZLLUM(.5));
	ZL_Display::FillRect(0, 0, FIELD_WIDTH, BLOCK_WIDTH*1000, ZLLUM(.1));
	srfBoxBG.DrawTo(0, 0, FIELD_WIDTH, BLOCK_WIDTH*1000, ZLLUM(.1));
	ZL_Display::FillRect(0, 0, (BLOCK_WIDTH/15), FIELD_HEIGHT, ZLLUM(.5));
	ZL_Display::FillRect(FIELD_WIDTH-(BLOCK_WIDTH/15), 0, FIELD_WIDTH, FIELD_HEIGHT, ZLLUM(.5));
	ZL_Display::FillRect(0, 0, FIELD_WIDTH, (BLOCK_WIDTH/15), ZLLUM(.5));
	ZL_Display::FillRect(-BLOCK_WIDTH, FIELD_HEIGHT, FIELD_WIDTH+BLOCK_WIDTH, FIELD_HEIGHT+0.2f, ZL_Color::Red);
	for (vector<sBlock*>::iterator itBlockDrawMain = Blocks.begin(); itBlockDrawMain != Blocks.end(); ++itBlockDrawMain) (*itBlockDrawMain)->Draw();
	for (vector<sBlock*>::iterator itBlockDrawLink = Blocks.begin(); itBlockDrawLink != Blocks.end(); ++itBlockDrawLink) (*itBlockDrawLink)->DrawLinks();
	Player->Draw();
	ZL_Display::PopOrtho();

	/*
	fntMain.Draw(10, ZLFROMH(20), ZL_String("Friction: ") << Player->body->shapeList->u);
	fntMain.Draw(10, ZLFROMH(40), ZL_String("Grounded: ") << (Player->grounded ? "YES" : "NO"));
	fntMain.Draw(10, ZLFROMH(60), ZL_String("Y-Vel: ") << Player->body->v.y);
	fntMain.Draw(10, ZLFROMH(80), ZL_String("Boost: ") << Player->remainingBoost);
	fntMain.Draw(10, ZLFROMH(100), ZL_String("LastJumpState: ") << (Player->lastJumpState ? "YES" : "NO"));
	fntMain.Draw(10, ZLFROMH(120), ZL_String("X-SurfaceVel: ") << Player->body->shapeList->surfaceV.x);
	*/
}
