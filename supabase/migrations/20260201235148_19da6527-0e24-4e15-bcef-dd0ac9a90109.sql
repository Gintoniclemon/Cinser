-- Add unique constraint on lottery_stats for upsert to work
ALTER TABLE public.lottery_stats 
ADD CONSTRAINT lottery_stats_game_stat_numero_unique 
UNIQUE (game_type, stat_type, numero);