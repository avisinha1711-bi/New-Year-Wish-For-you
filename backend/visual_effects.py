# backend/visual_effects.py
import colorsys
import math

class VisualEffectsGenerator:
    """Generate dynamic visual effects programmatically"""
    
    def generate_particle_effects(self, collision_data):
        """Generate custom particle effects for collisions"""
        
        effects = []
        
        # Different effects based on bubble type/size
        if collision_data['bubble_size'] > 20:
            effects.append(self._create_explosion_effect(collision_data))
        if collision_data['combo'] > 3:
            effects.append(self._create_combo_effect(collision_data))
        if collision_data['special']:
            effects.append(self._create_special_effect(collision_data))
        
        # Add screen shake for big explosions
        if collision_data['impact'] > 50:
            effects.append(self._create_screen_shake(collision_data))
        
        return effects
    
    def _create_explosion_effect(self, data):
        """Create explosion particle system"""
        particles = []
        x, y = data['x'], data['y']
        
        for i in range(15):
            angle = (i / 15) * 2 * math.pi
            speed = 2 + random.random() * 3
            life = 0.5 + random.random() * 0.5
            
            particles.append({
                'type': 'particle',
                'x': x,
                'y': y,
                'vx': math.cos(angle) * speed,
                'vy': math.sin(angle) * speed,
                'life': life,
                'color': self._explosion_color(i),
                'size': 3 + random.random() * 4
            })
        
        return {
            'type': 'explosion',
            'particles': particles,
            'duration': 1.0
        }
    
    def _explosion_color(self, index):
        """Generate explosion color gradient"""
        hue = (index / 15 + random.random() * 0.1) % 1.0
        rgb = colorsys.hsv_to_rgb(hue, 0.8, 1.0)
        return f'rgb({int(rgb[0]*255)},{int(rgb[1]*255)},{int(rgb[2]*255)})'
    
    def generate_background_theme(self, game_state):
        """Dynamically change background based on game state"""
        
        if game_state['speed_level'] > 15:
            return self._create_intense_theme()
        elif game_state['score'] > 10000:
            return self._create_achievement_theme()
        elif game_state['combo'] > 5:
            return self._create_combo_theme()
        
        return self._create_normal_theme()
