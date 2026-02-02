-- Create crescendo_tirages table for Crescendo game
CREATE TABLE public.crescendo_tirages (
  id UUID NOT NULL DEFAULT gen_random_uuid() PRIMARY KEY,
  date_tirage DATE NOT NULL,
  annee INTEGER NOT NULL,
  numero_1 INTEGER NOT NULL,
  numero_2 INTEGER NOT NULL,
  numero_3 INTEGER NOT NULL,
  numero_4 INTEGER NOT NULL,
  numero_5 INTEGER NOT NULL,
  created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(),
  UNIQUE(date_tirage)
);

-- Enable RLS on crescendo_tirages
ALTER TABLE public.crescendo_tirages ENABLE ROW LEVEL SECURITY;

-- Create policy for public read access (lottery data is public)
CREATE POLICY "Crescendo tirages are publicly readable" 
ON public.crescendo_tirages 
FOR SELECT 
USING (true);

-- Create index for faster queries
CREATE INDEX idx_crescendo_tirages_date ON public.crescendo_tirages(date_tirage DESC);
CREATE INDEX idx_crescendo_tirages_annee ON public.crescendo_tirages(annee);

-- Add realtime support
ALTER PUBLICATION supabase_realtime ADD TABLE public.crescendo_tirages;